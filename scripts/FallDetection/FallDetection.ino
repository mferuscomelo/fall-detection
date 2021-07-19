// Adapted from: https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/examples/hello_world/main_functions.cc

#include <Arduino_LSM9DS1.h> // For our sensors

#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/all_ops_resolver.h" // provides the operations used by the interpreter to run the model
#include "tensorflow/lite/micro/micro_error_reporter.h" // outputs debug information
#include "tensorflow/lite/micro/micro_interpreter.h" // contains code to load and run models
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h" // TODO: what does this do?
#include "tensorflow/lite/version.h" // provides versioning information for the TensorFlow Lite schema

#include "model.h" // Our model

// TFLite globals, used for compatibility with Arduino-style sketches
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  TfLiteTensor* model_output = nullptr;

  // Create an area of memory to use for input, output, and other TensorFlow
  // arrays. You'll need to adjust this by compiling, running, and looking
  // for errors.
  constexpr int kTensorArenaSize = 5 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
} // namespace

const int numSamples = 400;

int samplesRead = numSamples;

// array to map gesture index to a name
const char* ACTIONS[] = {
  "ADL",
  "Fall"
};

#define NUM_ACTIONS (sizeof(ACTIONS) / sizeof(ACTIONS[0]))

void setup() {

  Serial.begin(9600);

  // Wait for Serial to connect
  while (!Serial);

  // Initialize IMU
  initIMU();

  // Set up logging (will report to Serial, even within TFLite functions)
//  Serial.println("Set up logging");
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure
//  Serial.println("Map the model into a usable data structure");
  model = tflite::GetModel(model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model version does not match Schema");
    while(1);
  }
  
  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
//  Serial.println("pulls in all the operation implementations we need");
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model
//  Serial.println("Build an interpreter to run the model");
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize,
    error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
//  Serial.println("Allocate memory from the tensor_arena for the model's tensors");
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    while(1);
  }

  // Assign model input and output buffers (tensors) to pointers
//  Serial.println("Assign model input and output buffers (tensors) to pointers");
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);

  // Get information about the memory area to use for the model's input
  // Supported data types:
  // https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/c/common.h#L226
  Serial.print("Number of dimensions: ");
  Serial.println(model_input->dims->size);
  Serial.print("Input type: ");
  Serial.println(model_input->type);
}

void loop() {
  float aX, aY, aZ;

  // wait for significant motion
  while (samplesRead == numSamples) {
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX, aY, aZ);

      samplesRead = 0;

      break;
    }
  }

  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples) {
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);

      // store data in the model's input tensor
      model_input->data.f[0] = aX;
      model_input->data.f[1] = aY;
      model_input->data.f[2] = aZ;

      samplesRead++;

      if (samplesRead == numSamples) {
        // Run inference
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
          error_reporter->Report("Invoke failed on input");
        }

        // Loop through the output tensor values from the model
        for (int i = 0; i < NUM_ACTIONS; i++) {
          Serial.print(ACTIONS[i]);
          Serial.print(": ");
          Serial.println(model_output->data.f[i], 6);
        }
        Serial.println();
      }
    }
  }
}

void initIMU() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
}

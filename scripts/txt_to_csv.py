import pandas as pd
import os

dataset_dir = "./../dataset/"

for subdir, dirs, files in os.walk(dataset_dir):
    for file in files:
        if ("Readme" in file) or ("txt" not in file) or ("rar" in file):
            break

        txt_file_path = os.path.join(subdir, file)
        csv_file_path = txt_file_path.replace("txt", "csv")

        print(txt_file_path, "=>", csv_file_path)

        txt_file = pd.read_csv(str(txt_file_path))
        txt_file.to_csv(str(csv_file_path), index=None)

        os.remove(txt_file_path)
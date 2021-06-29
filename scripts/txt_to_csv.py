import os

dataset_dir = "./../dataset/"

def clean_file(path: str):
    """Remove semi-colons and spaces from files

    Args:
        path (str): Path of the file to clean
    """    
    blacklisted_chars = [";", " "]

    with open(path, "r+") as file:
        contents = file.read()

        for char in blacklisted_chars:
            contents = contents.replace(char, "")

        file.seek(0)
        file.write(contents)
        file.truncate()

for subdir, dirs, files in os.walk(dataset_dir):
    for file in files:
        if ("Readme" in file) or ("txt" not in file):
            continue

        txt_file_path = os.path.join(subdir, file)
        csv_file_path = txt_file_path.replace("txt", "csv")

        print(txt_file_path, "=>", csv_file_path)

        clean_file(txt_file_path)

        os.rename(txt_file_path, csv_file_path)
import os
import gzip
import shutil

def compress_file(file_path):
    """压缩单个文件为 .gz 格式，并删除原始文件"""
    # 构建压缩后的文件路径
    gz_path = file_path + ".gz"

    # 使用 gzip 压缩文件
    with open(file_path, "rb") as f_in:
        with gzip.open(gz_path, "wb") as f_out:
            shutil.copyfileobj(f_in, f_out)

    # 删除原始文件
    os.remove(file_path)
    print(f"Compressed and deleted: {file_path} -> {gz_path}")

def compress_folder(folder_path):
    """压缩文件夹内所有文件"""
    # 遍历文件夹内的所有文件
    for root, dirs, files in os.walk(folder_path):
        for file_name in files:
            file_path = os.path.join(root, file_name)

            # 跳过已经是 .gz 格式的文件
            if file_name.endswith(".gz"):
                print(f"Skipping already compressed file: {file_path}")
                continue

            # 压缩文件
            compress_file(file_path)

if __name__ == "__main__":
    # 指定要压缩的文件夹路径
    folder_path = "./data/_next"  # 替换为你的文件夹路径

    # 检查文件夹是否存在
    if not os.path.exists(folder_path):
        print(f"Error: Folder '{folder_path}' does not exist.")
    else:
        # 压缩文件夹内所有文件
        compress_folder(folder_path)
        print("Compression completed!")
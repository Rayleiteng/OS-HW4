import struct
import os

# 配置
FILENAME = "output.bin"
RECORD_SIZE = 128

def check_sorted(filename):
    if not os.path.exists(filename):
        print(f"Error: {filename} does not exist.")
        return

    file_size = os.path.getsize(filename)
    total_records = file_size // RECORD_SIZE
    print(f"Checking {filename}...")
    print(f"Total size: {file_size} bytes")
    print(f"Total records: {total_records}")

    if file_size % RECORD_SIZE != 0:
        print("Error: File size is not a multiple of 100!")
        return

    with open(filename, "rb") as f:
        # 读取第一条记录
        data = f.read(RECORD_SIZE)
        if not data:
            return
            
        # struct.unpack('<I', ...) 解析小端序 unsigned int (4 bytes)
        last_key = struct.unpack('<I', data[:4])[0]
        
        for i in range(1, total_records):
            data = f.read(RECORD_SIZE)
            current_key = struct.unpack('<I', data[:4])[0]
            
            # 核心检查：如果现在的 Key 比上一个小，说明没排好
            if current_key < last_key:
                print(f"❌ FAIL at index {i}:")
                print(f"   Previous key: {last_key}")
                print(f"   Current key:  {current_key}")
                return
            
            last_key = current_key

    print("✅ PASS: The file is correctly sorted!")

if __name__ == "__main__":
    check_sorted(FILENAME)
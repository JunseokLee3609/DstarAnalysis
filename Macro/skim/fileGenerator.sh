#!/bin/bash

# 사용법: ./organize_files.sh <입력_경로>
# 예시: ./organize_files.sh "/u/user/vince402/SE_UserHome/..."

# 입력 경로를 인자로 받습니다.
INPUT_PATH=$1

# 입력 경로가 비어있거나 존재하지 않으면 오류 메시지 출력 후 종료
if [ -z "$INPUT_PATH" ] || [ ! -d "$INPUT_PATH" ]; then
    echo "Error: Please provide a valid input directory path."
    echo "Usage: $0 <input_path>"
    exit 1
fi

# 파일 목록을 저장할 폴더 생성 (현재 디렉터리에 'file_lists' 폴더를 만듭니다)
OUTPUT_DIR="file_lists"
mkdir -p "$OUTPUT_DIR"

# 기존에 생성된 파일을 모두 삭제하여 깨끗한 상태로 시작
rm -f "$OUTPUT_DIR"/files*.txt
rm -f "combined_file_list.txt"

# 10개씩 묶어 파일 생성
find "$INPUT_PATH" -maxdepth 2 -type f -name "*.root" | awk -v n=10 -v outdir="$OUTPUT_DIR" '{
    file = outdir "/files" int((NR-1)/n)+1 ".txt";
    print $0 > file
}'

# 만들어진 모든 파일 목록을 하나의 파일로 묶기
find "$OUTPUT_DIR" -type f -name "files*.txt" | awk '{print $0, NR}' > "combined_file_list.txt"

echo "Files successfully processed."
echo "10-file lists are saved in the '$OUTPUT_DIR' directory."
echo "All file paths are combined into 'combined_file_list.txt'."

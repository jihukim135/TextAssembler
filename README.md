# TextAssembler
가상의 16bit 환경에서 디자인된 어셈블리를 바이너리 코드로 바꾸어 텍스트 파일에 출력하는 학습 목적의 프로그램입니다.
  
## 사용법
명령 프롬프트를 통해 실행하고 명령줄 인자로 어셈블하고자 하는 .asm 파일의 이름을 입력합니다. (ex: SampleSource.asm)  
바이너리 코드는 소스 파일과 이름이 같고 확장자가 .mac인 파일에 저장되며, 해당 파일이 없을 경우 새로 생성됩니다.  
`#define WRITE_LINE_BY_LINE`을 주석 처리하면 개행문자 없이 출력됩니다.
  
## 명령어 디자인
### 레지스터
레지스터는 16bit로 디자인되었으며, 총 8개의 레지스터를 사용한다고 가정합니다.  
![image](https://github.com/jihukim135/TextAssembler/assets/90246317/38c9c065-85e0-48ba-be41-69d9a7fbb4b8)  
  
각각의 레지스터 심볼은 아래와 같은 바이너리 코드에 대응됩니다.  
  
![image](https://github.com/jihukim135/TextAssembler/assets/90246317/d6dc4310-ceba-43ad-b56d-0657c8a36893)  

  
### 산술 연산
산술 연산 명령어는 공통적으로 다음과 같은 구조를 사용합니다.  
- 저장소 자리에는 무조건 레지스터만 올 수 있습니다.
- 피연산자에 레지스터가 올 경우, 상수(literal)와의 구분을 위해 최상위 비트에 1을 채워 넣습니다.  
- 피연산자로 올 상수의 표현 가능 범위는 0~7로 총 8개입니다.  
![image](https://github.com/jihukim135/TextAssembler/assets/90246317/715cb653-69d1-4fc6-86e7-9660aba08efb)
  
ADD, SUB, MUL, DIV 총 네 개의 연산을 지원합니다.  
![image](https://github.com/jihukim135/TextAssembler/assets/90246317/ec3eca4f-064c-4459-af84-448838765aaf)  
  
### LOAD/STORE 연산
LOAD는 메인 메모리의 주소 source로부터 레지스터 destination으로 16bit 데이터를 읽어 옵니다.  
STORE는 메인 메모리의 주소 destination에 레지스터 source의 16bit 데이터를 저장합니다.  
![image](https://github.com/jihukim135/TextAssembler/assets/90246317/c940f725-4f7e-4936-a726-f9a82c0236c8)  
![image](https://github.com/jihukim135/TextAssembler/assets/90246317/888def08-1d5b-48d6-a006-b3f2555ff7e3)  
   
LOAD와 STORE 모두 메인 메모리 주소를 대괄호`[]`로 감싸 Indirect Mode로 동작할 것을 명시할 수 있습니다.
Indirect Mode에서 동작하는 명령어는 바이너리 코드의 예약된 비트에 11을 채워 넣음으로써 구분됩니다.
  
### PUSH/POP 연산
'PUSH n'은 현재 스택에 1byte 크기의 데이터 n을 push하고 스택 포인터의 값을 증가시킵니다.  
POP은 스택 포인터의 값을 1byte 감소시킵니다.  
PUSH/POP 연산은 바이너리 코드와 1:1로 대응되지 않으며, 다른 여러 개의 명령어로 해석됩니다.  
  
## 샘플 코드
### .asm
```
MUL r1, 3, 3
MUL r2, 2, 2
ADD r3, r1, r2
STORE r3, 0x11
STORE r2, [0x11]
PUSH 5
POP 
```
### .mac
```
0001100100110011
0001101000100010
0000101110011010
0011101100010001
1111101000010001
0000100101010000
0011110101000000
1111100101000000
0000110111010001
0001010111010001
```
  
## 실행 환경
Windows 환경에서만 동작하며, MSVC로 컴파일한다고 가정합니다.
  
## 참고자료
- 윤성우의 뇌를 자극하는 윈도우즈 시스템 프로그래밍
- 청강문화산업대학교 시스템프로그래밍 강의
  
## 이미지 출처
- 윤성우의 뇌를 자극하는 윈도우즈 시스템 프로그래밍

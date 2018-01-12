#include <stdio.h>

#include <stdio.h>
#include <Windows.h>
#include <IMAGEHLP.H>
#pragma comment(lib,"ImageHlp.lib")

#define  FIND(struc,e) (unsigned int)&(((struc *)0)->e)

void MyCls(HANDLE hConsole)
{
    COORD   coordScreen = { 0, 0 };//�����������귵�ص���Ļ���Ͻ�����
    BOOL	bSuccess;
    DWORD   cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO   csbi;//���滺������Ϣ   
    DWORD   dwConSize;//��ǰ�����������ɵ��ַ���   
    bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);//��û�������Ϣ   
    dwConSize = csbi.dwSize.X*csbi.dwSize.Y;//�����������ַ���Ŀ   
    bSuccess = FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
    bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);//��û�������Ϣ   
    bSuccess = FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    bSuccess = SetConsoleCursorPosition(hConsole, coordScreen);
    return;
}

void clrscr(void)
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    MyCls(hStdOut);
    return;
}

DWORD FileLen(char *filename)
{
    WIN32_FIND_DATAA fileInfo = { 0 };
    DWORD fileSize = 0;
    HANDLE hFind;
    hFind = FindFirstFileA(filename, &fileInfo);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        fileSize = fileInfo.nFileSizeLow;
        FindClose(hFind);
    }
    return fileSize;
}

CHAR *LoadFile(char *filename)
{
    DWORD dwReadWrite, LenOfFile = FileLen(filename);
    HANDLE hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        printf("�ļ���С:0x%llx\n", LenOfFile);
        PCHAR buffer = (PCHAR)malloc(LenOfFile);
        SetFilePointer(hFile, 0, 0, FILE_BEGIN);
        ReadFile(hFile, buffer, LenOfFile, &dwReadWrite, 0);
        CloseHandle(hFile);
        return buffer;
    }
    return NULL;
}

VOID ShowPE64Info(char *filename)
{
    PIMAGE_NT_HEADERS64 pinths64;
    PIMAGE_DOS_HEADER pdih;
    char *filedata;
    DWORD i = 0;
    DWORD HeaderSum, CheckSum;
    HANDLE hWriteFile;
    DWORD dwFileLen;
    DWORD dwWriteLen;
    DWORD dwCheckSumOffset = FIND(IMAGE_OPTIONAL_HEADER64, CheckSum);

    filedata = LoadFile(filename);
    //����ļ����
    pdih = (PIMAGE_DOS_HEADER)filedata;
    //���ļ����ת��ΪDOSͷָ��
    pinths64 = (PIMAGE_NT_HEADERS64)(filedata + pdih->e_lfanew);
    //��ȡNTͷ
    if (pinths64->Signature != 0x00004550)//PE00
    {
        printf("��Ч��PE�ļ���\n");
        return;
    }
    if (pinths64->OptionalHeader.Magic != 0x20b)//�ж��ǲ���PE+��ʽ�ļ�
    {
        printf("����PE32+��ʽ���ļ���\n");
        return;
    }

    printf("\n");
    printf("��ڵ㣺          %llx\n", pinths64->OptionalHeader.AddressOfEntryPoint);
    printf("�����ַ��        %llx\n", pinths64->OptionalHeader.ImageBase);
    printf("�����С��        %llx\n", pinths64->OptionalHeader.SizeOfImage);
    printf("�����ַ��        %llx\n", pinths64->OptionalHeader.BaseOfCode);
    printf("����룺          %llx\n", pinths64->OptionalHeader.SectionAlignment);
    printf("�ļ�����룺      %llx\n", pinths64->OptionalHeader.FileAlignment);
    printf("��ϵͳ��          %llx\n", pinths64->OptionalHeader.Subsystem);
    printf("������Ŀ��        %llx\n", pinths64->FileHeader.NumberOfSections);
    printf("ʱ�����ڱ�־��    %llx\n", pinths64->FileHeader.TimeDateStamp);
    printf("�ײ���С��        %llx\n", pinths64->OptionalHeader.SizeOfHeaders);
    printf("����ֵ��          %llx\n", pinths64->FileHeader.Characteristics);
    printf("У��ͣ�          %llx\n", pinths64->OptionalHeader.CheckSum);
    printf("��ѡͷ����С��    %llx\n", pinths64->FileHeader.SizeOfOptionalHeader);
    printf("RVA ������С��    %llx\n", pinths64->OptionalHeader.NumberOfRvaAndSizes);

    printf("\n");
    dwFileLen = FileLen(filename);
    printf("PatchCheckSum do!\n");
    CheckSumMappedFile(filedata, dwFileLen, &HeaderSum, &CheckSum);
    printf("����У��ͣ�          %llx\n", CheckSum);
    printf("����ͷ��У��ͣ�          %llx\n", HeaderSum);

    pinths64->OptionalHeader.CheckSum = CheckSum;

    printf("У��ͣ�0x%x\n", pinths64->OptionalHeader.CheckSum);

    printf("WritrFile do!\n");

    hWriteFile = CreateFile(L"Patch.exe",
        GENERIC_WRITE | GENERIC_READ,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hWriteFile == INVALID_HANDLE_VALUE)
    {
        printf("WriteFile Faild!\n");
        return;
    }

    if (!WriteFile(hWriteFile, filedata, dwFileLen, &dwWriteLen, NULL))
    {
        printf("WriteFile Faild!  Error code:0x%x\n", GetLastError());
    }

    printf("WriteFile Success!\n");
    CloseHandle(hWriteFile);
}

void PrintCUI()
{
    char filename[MAX_PATH] = { 0 };
    SetConsoleTitleA("FixSumToolx64");
bgn:
    printf("FixSumToolx64\n");
    printf("�����ļ�����֧���ļ���ק)����exit�˳���:");
    gets(filename);
    if (FileLen(filename) == 0)
    {
        if (stricmp(filename, "exit"))
        {
            printf("��Ч���ļ�\n");
            goto invail;
        }
        else
            goto end;
    }
    ShowPE64Info(filename);
invail:
    getchar();
    clrscr();
    goto bgn;
end:
    return;
}

int main(int argc, char* argv[])
{
    PrintCUI();
    return 0;
}
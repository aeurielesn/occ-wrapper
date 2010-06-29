#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>

#define MAX_THREADS 3
#define BUF_SIZE 255

struct option
{
    const char *name;
    int has_arg;
    int val;
};

# define no_argument            0
# define required_argument      1
# define optional_argument      2

static struct option const long_options[] =
{
    {"initial-test-case", optional_argument, 'i'},
    {"test-case-step", optional_argument, 's'},
    {"number-of-test-cases", required_argument, 't'},
    {"execute", required_argument, 'x'},
    {"pre-execute", required_argument, 'p'},
    {"post-execute", required_argument, 'P'},
    {"test-cases-syntax", required_argument, 'y'},
    {"prog-id", required_argument, 'r'},
    {"os", required_argument, 'o'},
    {"time-limit", optional_argument, 'T'},
    {NULL, 0, 0}
};

DWORD WINAPI MyThreadFunction( LPVOID lpParam );
void ErrorHandler(LPTSTR lpszFunction);

typedef struct MyData {
    int cnt;
    char args[10][50];
} MYDATA, *PMYDATA;

int initial_test_case = 1,
    test_cases_step = 1,
    number_of_test_cases = 0,
    time_limit = 1000;

char *execute,
    *pre_execute,
    *post_execute,
    *test_cases_syntax,
    *out_file_name,
    *prog_id;

enum os {
    xp, vista
} os;

int is_long_option(char *arg)
{
    int ambigous = 0, ret = -1;
    char *buf;
    if(arg[0] != '\0')
    {
        if(arg[0] == '-' && arg[1] != '\0')
        {
            if(arg[0] == '-' && arg[1] == '-' && arg[2] != '\0')
            {
                buf = arg + 2;
                for(int i = 0; long_options[i].name != NULL ; ++i)
                {
                    if(strcmp(buf, long_options[i].name) == 0)
                    {
                        ret = long_options[i].val;
                        ambigous++;
                    }
                }
            }
        }
    }
    return (ambigous > 1)?-1:ret;
}

void check_arguments(int ini, int cnt, int &argc, char **argv)
{
    for(int i=ini+1; i<=ini+cnt; ++i)
    {
        if((i>=argc) || (is_long_option(argv[i])!=-1))
        {
            printf("'%s' has invalid arguments.\n", argv[ini]);
            exit(1);
        }
    }
}

void initialize(int &argc, char **argv)
{
    printf("argc: %d\n", argc);
    for(int i=1; i<argc; ++i)
    {
        int long_option = is_long_option(argv[i]);
        printf("[%d] %s %c\n", i, argv[i], long_option);

        switch(long_option)
        {
            case 'i': // initial-test-case
            case 's': // test-case-step
            case 'x': // execute
            case 't': // number-of-test-cases
            case 'p': // pre-execute
            case 'P': // post-execute
            case 'y': // test-cases-syntax
            case 'r': // prog-id
            case 'T': // time-limit
                check_arguments(i, 1, argc, argv);
                break;
            case 'o': // os
                check_arguments(i, 1, argc, argv);
                if(strcmp("xp", argv[i+1])!=0 && strcmp("vista", argv[i+1])!=0)
                    long_option = -1;
        }

        if(long_option == -1)
        {
            printf("'%s' is not a valid option.\n", argv[i]);
            exit(1);
        }

        switch(long_option)
        {
            case 'i': // initial-test-case
                initial_test_case = atoi(argv[++i]);
                break;
            case 's': // test-case-step
                test_cases_step = atoi(argv[++i]);
                break;
            case 'x': // execute
                execute = argv[++i];
                break;
            case 't': // number-of-test-cases
                number_of_test_cases = atoi(argv[++i]);
                break;
            case 'p': // pre-execute
                pre_execute = argv[++i];
                break;
            case 'P': // post-execute
                post_execute = argv[++i];
                break;
            case 'y': // test-cases-syntax
                test_cases_syntax = argv[++i];
                break;
            case 'r': // prog-id
                prog_id = argv[++i];
                break;
            case 'o': // os
                ++i;
                if(strcmp("xp", argv[i])==0)
                    os = xp;
                else if(strcmp("vista", argv[i])==0)
                    os = vista;
                break;
            case 'T': // time-limit
                time_limit = atoi(argv[++i]);
                break;
        }

    }
}

int _tmain(int argc, char **argv)
{
    char cmd[1000], current_test_case[1000];
    PMYDATA pData;
    DWORD dwThreadId;
    HANDLE hThread;

    initialize(argc, argv);

    // run test cases
    for( int i = 0; i < number_of_test_cases; ++i)
    {
        sprintf(current_test_case, test_cases_syntax, initial_test_case + i * test_cases_step);

        // data allocation
        pData = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));

        // allocation fails
        if( pData == NULL )
        {
            ExitProcess(2);
        }

        // thread data
        pData->cnt = 1;
        strcpy(pData->args[0], execute);

        // run pre-execute
        sprintf(cmd, "%s %s %s", pre_execute, prog_id, current_test_case);
        printf("%s\n", cmd);
        system(cmd);

        // execute thread
        hThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            MyThreadFunction,       // thread function name
            pData,                  // argument to thread function
            0,                      // use default creation flags
            &dwThreadId);           // returns the thread identifier

        // check returned value
        if (hThread == NULL)
        {
           ErrorHandler(TEXT("CreateThread"));
           ExitProcess(3);
        }

        // wait execution
        DWORD r = WaitForSingleObject(hThread, (DWORD) time_limit);

        // clean process
        TerminateProcess(hThread, 1);
        if(os == vista)
        {
            sprintf(cmd, "taskkill /F /IM %s /T", execute);
            printf("%s\n", cmd);
            system(cmd);
        }
        else if(os == xp)
        {
            sprintf(cmd, "tskill %s", execute);
            printf("%s\n", cmd);
            system(cmd);
        }

        CloseHandle(hThread);
        if(pData != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pData);
            pData = NULL;
        }

        // run post-execute
        sprintf(cmd, "%s %s %s", post_execute, prog_id, current_test_case);
        printf("%s\n", cmd);
        system(cmd);
    }

    return 0;
}


DWORD WINAPI MyThreadFunction( LPVOID lpParam )
{
    HANDLE hStdout;
    PMYDATA pData;

    TCHAR msgBuf[BUF_SIZE];
    size_t cchStringSize;
    DWORD dwChars;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if( hStdout == INVALID_HANDLE_VALUE )
        return 1;

    pData = (PMYDATA)lpParam;

    printf("%s\n", pData->args[0]);
    system(pData->args[0]);

    return 0;
}



void ErrorHandler(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code.
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message.
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR));
    MessageBox(NULL, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

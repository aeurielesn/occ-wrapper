/***
*
* Written by [Alexander Urieles][aeurielesn].
*
* Based on a couple of files from [coreutils].
*
*   [aeurielesn]: http://github.com/aeurielesn
*   [coreutils]: http://www.gnu.org/software/coreutils/
*
***/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>

#define MAX_STR_LEN 100
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
    {"number-of-test-cases", optional_argument, 't'},
    {"execute", required_argument, 'x'},
    {"pre-execute", optional_argument, 'p'},
    {"post-execute", optional_argument, 'P'},
    {"test-cases-syntax", optional_argument, 'y'},
    {"problem-id", required_argument, 'r'},
    {"os", required_argument, 'o'},
    {"time-limit", optional_argument, 'T'},
    {NULL, 0, 0}
};

DWORD WINAPI ExecutionThread( LPVOID lpParam );
void ErrorHandler(LPTSTR lpszFunction);

int initial_test_case = 1,
    test_cases_step = 1,
    number_of_test_cases = 10,
    time_limit = 1000;

char execute[MAX_STR_LEN],
    pre_execute[MAX_STR_LEN],
    post_execute[MAX_STR_LEN],
    test_cases_syntax[MAX_STR_LEN],
    problem_id[MAX_STR_LEN];

enum os {
    xp, vista
} os;

enum option_type {
    none, short_option, long_option
};

int get_option(char *arg, option_type *type, char *value)
{
    int ambigous = 0, ret = -1;
    char buf[100], buf2[100];

    if(type != NULL)
        *type = none;

    if(arg[0] != '\0')
    {
        if((arg[0] == '-') && (arg[1] != '\0'))
        {
            if(arg[1] == '-')
            {
                sscanf(arg, "--%[^=]=%s", buf, buf2);
                for(int i = 0; long_options[i].name != NULL ; ++i)
                {
                    if(strcmp(buf, long_options[i].name) == 0)
                    {
                        if(type != NULL)
                            *type = long_option;
                        if(value != NULL)
                            strcpy(value, buf2);
                        ret = long_options[i].val;
                        ambigous++;
                    }
                }
            }
            else if(arg[1] != '-')
            {
                sscanf(arg, "-%s", buf);
                for(int i = 0; long_options[i].name != NULL ; ++i)
                {
                    if(buf[0] == long_options[i].val)
                    {
                        if(type != NULL)
                            *type = short_option;
                        ret = long_options[i].val;
                        ambigous++;
                    }
                }
            }
        }
    }
    return (ambigous > 1)?-1:ret;
}

void check_short_option_argument(int i, int &argc, char **argv)
{
    if((i>=argc) || (get_option(argv[i], NULL, NULL)!=-1))
    {
        printf("'%s' is not a valid argument.\n", argv[i]);
        exit(1);
    }
}

void initialize(int &argc, char **argv)
{
    char buf[100];
    option_type curr_type;
    for(int i=1; i<argc; ++i)
    {
        int curr_option = get_option(argv[i], &curr_type, buf);
//        printf("[%d] %s %c %s %d\n", i, argv[i], curr_option, buf, curr_type);

        if(curr_type == short_option)
        {
            check_short_option_argument(i+1, argc, argv);
            strcpy(buf, argv[++i]);
            switch(curr_option)
            {
                case 'o': // os
                    if(strcmp("xp", argv[i+1])!=0 && strcmp("vista", argv[i+1])!=0)
                        curr_option = -1;
            }
        }

        if(curr_option == -1)
        {
            printf("'%s' is not a valid option.\n", argv[i]);
            exit(1);
        }

        switch(curr_option)
        {
            case 'i': // initial-test-case
                initial_test_case = atoi(buf);
                break;
            case 's': // test-case-step
                test_cases_step = atoi(buf);
                break;
            case 'x': // execute
                strcpy(execute, buf);
                break;
            case 't': // number-of-test-cases
                number_of_test_cases = atoi(buf);
                break;
            case 'p': // pre-execute
                strcpy(pre_execute, buf);
                break;
            case 'P': // post-execute
                strcpy(post_execute, buf);
                break;
            case 'y': // test-cases-syntax
                strcpy(test_cases_syntax, buf);
                break;
            case 'r': // problem-id
                strcpy(problem_id, buf);
                break;
            case 'o': // os
                if(strcmp("xp", buf)==0)
                    os = xp;
                else if(strcmp("vista", buf)==0)
                    os = vista;
                break;
            case 'T': // time-limit
                time_limit = atoi(buf);
                break;
        }

    }
}

int _tmain(int argc, char **argv)
{
    char cmd[1000], current_test_case[1000];
    DWORD dwThreadId;
    HANDLE hThread;

    initialize(argc, argv);

    // run test cases
    for( int i = 0; i < number_of_test_cases; ++i)
    {
        sprintf(current_test_case, test_cases_syntax, initial_test_case + i * test_cases_step);

        // run pre-execute
        sprintf(cmd, "%s %s %s", pre_execute, problem_id, current_test_case);
        printf("%s\n", cmd);
        system(cmd);

        // execute thread
        hThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            ExecutionThread,       // thread function name
            NULL,                   // argument to thread function
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

        // run post-execute
        sprintf(cmd, "%s %s %s", post_execute, problem_id, current_test_case);
        printf("%s\n", cmd);
        system(cmd);
    }

    return 0;
}


DWORD WINAPI ExecutionThread( LPVOID lpParam )
{
    HANDLE hStdout;

    TCHAR msgBuf[BUF_SIZE];
    size_t cchStringSize;
    DWORD dwChars;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if( hStdout == INVALID_HANDLE_VALUE )
        return 1;

    printf("%s\n", execute);
    system(execute);

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

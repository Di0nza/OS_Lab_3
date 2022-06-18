#include <windows.h>
#include <iostream>
#include <vector>

int arrSize;
int *arr;
int markerSize;
HANDLE *_cantContinue;
HANDLE *_stopT;
HANDLE *_continueT;
HANDLE _begin;
CRITICAL_SECTION cs;

void WINAPI marker(int num) {
    WaitForSingleObject(_begin, INFINITY);
    srand(num);
    std::vector<int> labeled;
    while (true) {
        int i = rand() % arrSize;
        if (arr[i] == 0) {
            labeled.push_back(i);
            Sleep(5);
            arr[i] = num;
            Sleep(5);
        } else {
            EnterCriticalSection(&cs);
            std::cout << "Thread: " << num << std::endl << "Elements labeled: " << i << std::endl << "Unable to label: "
                      << i << std::endl << std::endl;;
            LeaveCriticalSection(&cs);
            SetEvent(_cantContinue[num]);


            HANDLE _events[2];

            _events[0] = _stopT[num];
            _events[1] = _continueT[num];
            WaitForMultipleObjects(2, _events, FALSE, INFINITE);
            if (WaitForSingleObject(_stopT[num], 0) == WAIT_OBJECT_0) {
                for (int j = 0; j < labeled.size(); j++) {
                    arr[labeled[j]] = 0;
                }
                return;
            }
            ResetEvent(_continueT[num]);
            ResetEvent(_cantContinue[num]);
        }
    }
}

void print(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}

int main() {
    InitializeCriticalSection(&cs);
    srand(time(0));
    std::cout << "Enter arr size: " << std::endl;
    std::cin >> arrSize;
    arr = new int[arrSize];
    for (int i = 0; i < arrSize; i++) {
        arr[i] = 0;
    }
    std::cout << "Enter amount of threads: " << std::endl;

    std::cin >> markerSize;
    HANDLE *_threads = new HANDLE[markerSize];
    DWORD *IDs = new DWORD[markerSize];
    _cantContinue = new HANDLE[markerSize];
    _stopT = new HANDLE[markerSize];
    _continueT = new HANDLE[markerSize];
    _begin = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    for (int i = 0; i < markerSize; i++) {
        _cantContinue[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        _stopT[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        _continueT[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        _threads[i] = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) marker, (LPVOID) i, 0, &IDs[i]);
    }

    SetEvent(_begin);

    while (WaitForMultipleObjects(markerSize, _threads, TRUE, 0) != WAIT_OBJECT_0) {
        WaitForMultipleObjects(markerSize, _cantContinue, TRUE, INFINITE);
        EnterCriticalSection(&cs);
        print(arr, arrSize);
        int finishT;
        std::cout << "Choose thread which should be finished:" << std::endl;
        std::cin >> finishT;
        if (finishT < 0 || finishT > markerSize - 1) {
            std::cout << "No such stream exists!" << std::endl;
            system("pause");
        }
        LeaveCriticalSection(&cs);
        SetEvent(_stopT[finishT]);
        WaitForSingleObject(_threads[finishT], INFINITE);
        for (int i = 0; i < arrSize; i++) {
            if (i != finishT) {
                SetEvent(_continueT[i]);
            }
        }
        EnterCriticalSection(&cs);
        print(arr, arrSize);
        LeaveCriticalSection(&cs);
    }
    system("pause");
    return 0;
}
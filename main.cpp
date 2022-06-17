#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>
CRITICAL_SECTION cs;
std::vector<HANDLE> tArray;
std::vector<HANDLE> tEvents;
HANDLE _begin;

struct UsefulData{
    int size;
    int* arr;
    int num;
    HANDLE _events[2];
    UsefulData(int size1, int* arr1, int num1) : size(size1), arr(arr1), num(num1){
        _events[0] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        _events[1] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }
};

UINT WINAPI marker (void* p){
    UsefulData* arg = static_cast<UsefulData*>(p);
    std::cout<< "Signal";
    WaitForSingleObject(_begin, INFINITE);
    std::vector<int> labeled;
    std::srand(arg -> num);
    while(true){
        EnterCriticalSection(&cs);
        int i = rand()%arg->num;
        if(arg->arr[i] == 0){
            Sleep(5);
            arg->arr[i] = arg->num;
            labeled.push_back(i);
            Sleep(5);
        }else{
            std::cout << "Thread  " << arg ->num << " labeled: " << labeled.size() << " Unable to label: " << i;
            LeaveCriticalSection(&cs);
            SetEvent(tEvents[arg -> num-1]);
            if(WaitForMultipleObjects(2, arg->_events, FALSE, INFINITE) - WAIT_OBJECT_0 == 1){
                for(int j = 0; j < labeled.size(); j++){
                    arg->arr[j] = 0;
                }
                break;
            }
        }
        LeaveCriticalSection(&cs);
    }
    return 0;
}

void print(int* arr, int size){
    EnterCriticalSection(&cs);
    for(int i = 0; i < size; i++){
        std::cout<<arr[i];
        std::cout<<" ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<UsefulData> data;
    srand(time(0));
    _begin = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    std::cout << "Enter arr arrSize: " << std::endl;
    int arrSize;
    std:: cin >> arrSize;
    int* arr = new int[arrSize];
    for(int i = 0; i < arrSize; i ++){
        arr[i] = 0;
    }

    std::cout<<"Enter amount of threads: "<< std:: endl;
    int markerSize;
    std::cin>>markerSize;
    bool *isProcess = new bool[markerSize];
    HANDLE _thread;
    for(int i = 0; i < markerSize; i ++){
        UsefulData* ud = new UsefulData(arrSize, arr, i+1);
        _thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, marker, ud, 0, nullptr));
        if(_thread == 0){
            std::cout<<"Error: Process not created"<<std::endl;
            return GetLastError();
        }
        tEvents.push_back(CreateEvent(nullptr, TRUE, FALSE, nullptr));
        if(tEvents[i] == nullptr){
            return GetLastError();
        }
        isProcess[i] = true;
        data.push_back(*ud);
    }
    InitializeCriticalSection(&cs);
    int finish;
    for(int i = 0; i != markerSize; i++){
        WaitForMultipleObjects(markerSize, &tEvents[0], TRUE, INFINITE);
        print(arr,arrSize);
        std::cout<<"Choose thread which should be finished:" << std::endl;
        std::cin>> finish;
        if(isProcess[finish] == true){
            isProcess[finish-1]=false;
            SetEvent(data[finish-1]._events[1]);
            WaitForSingleObject(tArray[finish-1], INFINITE);
            print(arr, arrSize);
            for(int j = 0; j < markerSize; j++){
                if(!isProcess[j]){
                    ResetEvent(tEvents[j]);
                    SetEvent(data[j]._events[0]);
                }
            }
        }
    }
    DeleteCriticalSection(&cs);
    return 0;
}

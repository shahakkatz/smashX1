#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout<<"smash: got ctrl-Z"<<endl;
	SmallShell &smash= SmallShell::getInstance();
	if(smash.Job_in_fg== nullptr){
        return;
	}
	smash.backgroundJobs->addJob(smash.Job_in_fg->command,smash.Job_in_fg->GetProcessID(), true);
    kill(smash.Job_in_fg->GetProcessID(),SIGSTOP);
    cout<<"smash: process "<<smash.Job_in_fg->GetProcessID()<<" was stopped"<<endl;
    smash.Job_in_fg= nullptr;
}

void ctrlCHandler(int sig_num) {
    cout<<"smash: got ctrl-C"<<endl;
    SmallShell &smash= SmallShell::getInstance();
    if(smash.Job_in_fg== nullptr){
        return;
    }
    kill(smash.Job_in_fg->GetProcessID(),SIGKILL);
    cout<<"smash: process "<<smash.Job_in_fg->GetProcessID()<<" was killed"<<endl;
    smash.Job_in_fg= nullptr;
}


void alarmHandler(int sig_num) {
  cout<<"smash got an alarm"<<endl;
}


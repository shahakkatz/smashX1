#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#define BASHPATH "/bin/bash"
#define MAX_SIZE 200

using namespace std;
const std::string WHITESPACE=" \n\r\t\f\v";
#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() {
    Prompt="smash";
    previosdir[0]=0;
    backgroundJobs= new JobsList;
    Job_in_fg= nullptr;
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    char last_char = *(cmd_s.end());
    if (firstWord.compare("chprompt") == 0) {
        return new ChangePrompt(cmd_line,this);
    }
    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line,this);
    }
    else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line,this);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line,this);
    }
    else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line,this);
    }
    else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line,this);
    }
    else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line,this);
    }
    else if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(cmd_line,this);
    }
    else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line,this);
    }
    else {
        return new ExternalCommand(cmd_line, this);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    if(cmd == nullptr) {
        perror("smash error: create command failed");
        return;
    }
   cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

ChangePrompt::ChangePrompt(const char *cmdLine, SmallShell *sheli) : BuiltInCommand(cmdLine,sheli) {


}

void ChangePrompt::execute() {
    if(this->NumOfArgs==1){
        this->MyShell->Prompt="smash";
        return;
    }
    this->MyShell->Prompt=this->args[1];

}

Command::Command(const char *cmd_line,SmallShell* sheli) :MyShell(sheli){
    c_line = (char *)(malloc(strlen(cmd_line) + 1));
    strcpy(c_line,cmd_line);
    isBackground= _isBackgroundComamnd(cmd_line);
    _removeBackgroundSign(c_line);
    args= (char **)(malloc(20 * sizeof(char *)));
    NumOfArgs= _parseCommandLine(c_line,args);
    pid=0;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line,SmallShell* sheli) : Command(cmd_line,sheli) {

}

ExternalCommand::ExternalCommand(const char *cmd_line, SmallShell *sheli) : Command(cmd_line, sheli) {

}

void GetCurrDirCommand::execute() {
    char pwd[256];
    getcwd(pwd,256);
    cout<<pwd<<"\n";

}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line,SmallShell* sheli) : BuiltInCommand(cmd_line,sheli) {

}
ChangeDirCommand::ChangeDirCommand(const char *cmdLine, SmallShell* sheli)
        : BuiltInCommand(cmdLine,sheli) {

        strcpy(prevdir,MyShell->previosdir);
}

void ChangeDirCommand::execute() {
    if ((NumOfArgs >= 2)) {
        perror("smash error: cd: too many arguments");
        return;
    }
    if(strcmp(this->args[1],"-")==0)
        if(prevdir[0]!=0) {
            strcpy(prevdir,MyShell->previosdir);
            getcwd(MyShell->previosdir, 1024);
            chdir(prevdir);


        }
        else
            perror("smash error: cd: OLDPWD not set");
    else {
        getcwd(MyShell->previosdir, 1024);
        chdir(args[1]);

    }
}

JobsList::JobEntry::JobEntry(int JobId, Command *command, pid_t processID) : JobId(JobId),command(command),ProcessID(processID){
        this->SecondsElapsed= time(nullptr);
        this->IsFinished= false;
        this->IsStopped= false;

}

void JobsList::JobEntry::StopJob() {
    this->IsStopped= true;

}

void JobsList::JobEntry::FinishedJob() {
    this->IsFinished=true;

}

int JobsList::JobEntry::GetJobID() {
    return this->JobId;
}

std::ostream &operator<<(ostream &o, const JobsList::JobEntry &Job) {
    if(!Job.IsStopped)
        return o<<'['<<Job.JobId<<']'<<Job.command->c_line<<" : "<<Job.ProcessID<<" "<<Job.getDuration()<<"\n";
    else
        return o<<'['<<Job.JobId<<']'<<Job.command->c_line<<" : "<<Job.ProcessID<<" "<<Job.SecondsElapsed<<" stopped"<<"\n" ;
}

int JobsList::JobEntry::GetProcessID() {
    return this->ProcessID;
}

char *JobsList::JobEntry::GetJobCommandLine() {
    return this->command->c_line;
}

time_t JobsList::JobEntry::getDuration() const {
    return time(0) - SecondsElapsed;
}

JobsList::JobsList() :MaxJobId(0){
    this->EntryJobs= new std::vector<JobEntry>;

}

JobsList::~JobsList() {
    free(this->EntryJobs);

}

void JobsList::addJob(Command *cmd, pid_t pid, bool isStopped) {
    JobEntry *NewJob= new JobEntry(this->MaxJobId+1,cmd,pid);
    NewJob->IsStopped=isStopped;
    MaxJobId++;
    this->EntryJobs->push_back(*NewJob);

}

void JobsList::printJobsList() {
    if(EntryJobs->empty()) {
        return;
    }
    auto it= this->EntryJobs->begin();
    while(it!=this->EntryJobs->end()){
        std::cout<<*(it);
        it++;
    }

}

void JobsList::killAllJobs() {
    auto it= this->EntryJobs->begin();
    while(it!=this->EntryJobs->end()){
        pid_t pid=it->command->pid;
        kill(pid,SIGKILL);
        it++;
    }

}

void JobsList::removeFinishedJobs() {
    auto it= this->EntryJobs->begin();
    while(it!=EntryJobs->end()){
        int result= waitpid(it->command->pid,NULL,WNOHANG);
        if (result>0)
            EntryJobs->erase(it);
        if(result==-1){
            perror("smash error: waitpid failed!");
            return;
        }
        it++;

    }

}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    auto it= this->EntryJobs->begin();
    while(it!=EntryJobs->end()){
        if (it->GetJobID()==jobId)
            return &(*it);
        it++;

    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    auto it= this->EntryJobs->begin();
    while(it!=EntryJobs->end()){
        if (it->GetJobID()==jobId) {
            EntryJobs->erase(it);
            return;
        }
        it++;

    }
}


void ExternalCommand::execute() {
    int status;
    int pid=fork();
    if (pid==-1){
        perror("smash error: fork failed!");
        return;
    }
    if (pid!=0){
        if(!isBackground){
            auto* Job = new JobsList::JobEntry(MyShell->backgroundJobs->MaxJobId+1,this,pid);
            MyShell->Job_in_fg=Job;
            MyShell->backgroundJobs->MaxJobId++;
            if(waitpid(pid, nullptr, WUNTRACED)==-1){
                perror("smash error: wait failed!");
            }
            MyShell->Job_in_fg= nullptr;
            return;
        }else{
            MyShell->backgroundJobs->addJob(this, pid);
            return;
        }
    }else{
        char exec_line[MAX_SIZE];
        strcpy(exec_line, c_line);
        if(exec_line[0] != '\000') {
            _removeBackgroundSign(exec_line);
        }
        char* TheRealArgs[]={(char*)BASHPATH, (char*)"-c", exec_line, nullptr};
        execv(TheRealArgs[0],TheRealArgs);
        perror("smash error: execv failed!");
        exit (1);
    }


}

void QuitCommand::execute() {
    this->MyShell->backgroundJobs->removeFinishedJobs();
    if(this->args[1]=="kill"){
        int n=this->MyShell->backgroundJobs->EntryJobs->size();
        cout<<"smash: sending SIGKILL signal to "<<n<<" jobs:"<<endl;
        auto it= this->MyShell->backgroundJobs->EntryJobs->begin();
        while(it!=MyShell->backgroundJobs->EntryJobs->end()){
            cout<<it->GetProcessID()<<": "<<it->GetJobCommandLine()<<endl;
            kill(it->GetProcessID(),SIGKILL);
            it++;
        }
    }
    exit(0);

}

QuitCommand::QuitCommand(const char *cmd_line, SmallShell *sheli): BuiltInCommand(cmd_line,sheli){


}

void JobsCommand::execute() {
    this->MyShell->backgroundJobs->removeFinishedJobs();
    this->MyShell->backgroundJobs->printJobsList();

}

JobsCommand::JobsCommand(const char *cmd_line, SmallShell *sheli) : BuiltInCommand(cmd_line, sheli){

}

void KillCommand::execute() {
    auto it = this->MyShell->backgroundJobs->getJobById(stoi(this->args[2]));
    if (!it){
        string s = this->args[2];
        string error= "smash error: kill: job-id "+s+" does not exist";
        perror(error.c_str());
        return;
    }
    ///*********************** this check is not perfect ************************///
    if (this->NumOfArgs>2){
        perror("smash error: kill: invalid arguments");
    }
    kill( it->GetProcessID(),stoi(this->args[1]));
    it->FinishedJob();
    MyShell->backgroundJobs->removeFinishedJobs();

}

KillCommand::KillCommand(const char *cmd_line, SmallShell *sheli) : BuiltInCommand(cmd_line, sheli) {

}

ShowPidCommand::ShowPidCommand(const char *cmd_line, SmallShell *sheli) : BuiltInCommand(cmd_line, sheli) {

}

void ShowPidCommand::execute() {
    if(!this->pid){
        perror("smash error: get pid failed!");
        return;
    }
    cout<<"smash pid is "<<this->pid<<endl;

}

ForegroundCommand::ForegroundCommand(const char *cmd_line, SmallShell *sheli) : BuiltInCommand(cmd_line, sheli) {

}

void ForegroundCommand::execute() {
    /// insert checks here///
    int* status;
    auto it = MyShell->backgroundJobs->EntryJobs->begin();
    if (this->NumOfArgs==2) {
        while (it != MyShell->backgroundJobs->EntryJobs->end()) {
            if (it->GetJobID() == stoi(this->args[1])) {
                cout<<it->command->c_line<<it->getDuration()<<endl;
                MyShell->Job_in_fg= MyShell->backgroundJobs->getJobById(it->GetJobID());
                MyShell->backgroundJobs->removeJobById(it->GetJobID());
                waitpid(it->command->pid,status,WUNTRACED);
                return;

            }

            it++;
        }
        string s= this->args[1];
        string error="smash error: fg: job-id "+s+ "does not exist";
        perror(error.c_str());
    }else if(this->NumOfArgs==1){
        if(MyShell->backgroundJobs->EntryJobs->empty()){
            perror("smash error: fg: jobs list is empty");
            return;
        }
        int max=MyShell->backgroundJobs->MaxJobId;
        JobsList::JobEntry* temp=MyShell->backgroundJobs->getJobById(max);
        cout<<temp->command->c_line<<" "<<temp->getDuration()<<endl;
        waitpid(temp->command->pid,NULL, WUNTRACED);
        MyShell->Job_in_fg=temp;
        MyShell->backgroundJobs->removeJobById(max);
    } else
        perror("smash error: fg: invalid arguments");

}

BackgroundCommand::BackgroundCommand(const char *cmd_line, SmallShell* sheli): BuiltInCommand(cmd_line,sheli) {

}

void BackgroundCommand::execute() {
    if (this->NumOfArgs==1){
        auto it = this->MyShell->backgroundJobs->EntryJobs->begin();
        while (it!=this->MyShell->backgroundJobs->EntryJobs->end()){
            if(it->IsStopped){
                it->IsStopped=0;
                cout<<it->command->c_line<<" : "<<it->getDuration()<<endl;
                kill(it->command->pid,SIGCONT);
                MyShell->backgroundJobs->removeJobById(it->GetJobID());
                return;
            }
            it++;
        }
        perror("smash error: bg: there is no stopped jobs to resume");
        return;
    }else if(this->NumOfArgs==2) {
        int JobId=stoi(this->args[0]);
        auto it = this->MyShell->backgroundJobs->EntryJobs->begin();
        while (it != this->MyShell->backgroundJobs->EntryJobs->end()) {
            if (it->GetJobID()==JobId ){
                if(!it->IsStopped){
                    string s= this->args[1];
                    string error= "smash error: bg: job-id "+s+"is already running in the background";
                    perror(error.c_str());
                    return;
                }else{
                    it->IsStopped=0;
                    cout<<it->command->c_line<<" : "<<it->getDuration()<<endl;
                    kill(it->GetProcessID(),SIGCONT);
                    MyShell->backgroundJobs->removeJobById(JobId);
                    return;
                }
            }
            it++;
        }
        string s= this->args[1];
        string error= "smash error: bg: job-id "+s+"does not exist";
        perror(error.c_str());
        return;

    }
}



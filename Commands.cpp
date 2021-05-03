#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>
#include <wait.h>
#include <stdlib.h>
#include <fstream>

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
int isRedirectOrPipe(string cmd_line);

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
    int type= isRedirectOrPipe(cmd_line);
    if(type!=0){
        if(type==1||type==2)
            return new RedirectionCommand(cmd_line,this,type);
        if(type==3||type==4)
            return new PipeCommand(cmd_line, this,type);

    }
    if (firstWord.compare("chprompt") == 0) {
        return new ChangePrompt(cmd_line,this);
    }
    if (firstWord.compare("cat") == 0) {
        return new CatCommand(cmd_line,this);
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
        //return new BackgroundCommand(cmd_line,this);
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

JobsList::JobEntry::JobEntry(int JobId, Command *command, int processID) : JobId(JobId),command(command),ProcessID(processID){
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
    int ProcessID= pid;
    JobEntry *NewJob= new JobEntry(this->MaxJobId+1,cmd,ProcessID);
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
        pid_t pid=it->GetProcessID();
        kill(pid,SIGKILL);
        it++;
    }

}

void JobsList::removeFinishedJobs() {
    auto it= this->EntryJobs->begin();
    while(it!=EntryJobs->end()){
        int result= waitpid(it->GetProcessID(),NULL,WNOHANG);
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
            wait(&status);
            if(status==-1){
                perror("smash error: wait failed!");
            }
            return;
        }else{
            MyShell->backgroundJobs->addJob(this, pid);
            return;
        }
    }else{
        char exec_line[MAX_SIZE];
        _trim(c_line);
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
        perror("smash error: kill: job-id <job-id> does not exist");
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
    auto it = MyShell->backgroundJobs->EntryJobs->begin();
    if (this->NumOfArgs==1) {
        while (it != MyShell->backgroundJobs->EntryJobs->end()) {
            if (it->GetJobID() == stoi(this->args[1])) {
                it->command->execute();
                cout<<it->command->pid<<it->command->c_line<<it->SecondsElapsed<<endl;
                MyShell->backgroundJobs->removeJobById(it->GetJobID());
                return;

            }

            it++;
        }
    }else if(this->NumOfArgs==0){
        int max=it->GetJobID();
        while (it != MyShell->backgroundJobs->EntryJobs->end()){
            if(it->GetJobID()>max)
                max=it->GetJobID();
            it++;
        }
        MyShell->backgroundJobs->getJobById(max)->command->execute();
        cout<<it->command->pid<<it->command->c_line<<it->SecondsElapsed<<endl;
        MyShell->backgroundJobs->removeJobById(max);
    }

}
int isRedirectOrPipe(string cmd_line){
    size_t found=cmd_line.find(">>");
    if(found!=string::npos) {
        /** APPEND **/
        return 2;
    }
    found=cmd_line.find(">");
    if(found!=string::npos){
        /** NEW TEXT FILE **/
        return 1;

    }
    found=cmd_line.find("|&");
    if(found!=string::npos) {
        /** ERR PIPE **/
        return 4;
    }
    found=cmd_line.find('|');
    if(found!=string::npos) {
        /** PIPE **/
        return 3;
    }

    return 0;

}

RedirectionCommand::RedirectionCommand(const char *cmdLine, SmallShell *sheli, int type) : Command(cmdLine, sheli) {
    TypeOfRed=type;

}

void RedirectionCommand::execute() {
    string line_cmd(this->c_line);
    string fileName = _trim(line_cmd.substr(line_cmd.find_first_of(">|") + 2));
    int flags;
    if(this->TypeOfRed==1) flags=(O_RDWR | O_CREAT | O_TRUNC);
    if(this->TypeOfRed==2) flags=(O_WRONLY | O_APPEND | O_CREAT);
    int stdout_copy = dup(STDOUT_FILENO);
    int fd = open(fileName.c_str(), flags, 0666);
    dup2(fd, STDOUT_FILENO);
    string NewCmd = line_cmd.substr(0, line_cmd.find_first_of('>'));
    Command *cmd = this->MyShell->CreateCommand(NewCmd.c_str());
    cmd->execute();
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    close(fd);
}
PipeCommand::PipeCommand(const char *cmd_line, SmallShell *sheli,int type) : Command(cmd_line, sheli) {
    this->Type=type;
}

void PipeCommand::execute() {
    int out;
    if(this->Type==3) out= STDOUT_FILENO;
    if (this->Type==4) out= STDERR_FILENO;
    int fd[2];
    pipe(fd);
    string cmd_line(c_line);
    string First=_trim(cmd_line.substr(0,cmd_line.find('|')));
    string Second=_trim(cmd_line.substr(cmd_line.find('|')+1));
    int stdout_copy=dup(STDOUT_FILENO);
    dup2(fd[1],STDOUT_FILENO);
    Command* Left=this->MyShell->CreateCommand(First.c_str());
    Left->execute();
    dup2(stdout_copy,STDOUT_FILENO);
    close(stdout_copy);
    close(fd[1]);
    int stdin_copy=dup(STDIN_FILENO);
    dup2(fd[0],STDIN_FILENO);
    Command* Right=this->MyShell->CreateCommand(Second.c_str());
    Right->execute();
    dup2(stdin_copy,STDIN_FILENO);
    close(stdin_copy);
    close(fd[0]);
}


CatCommand::CatCommand(const char *cmd_line, SmallShell *sheli) : BuiltInCommand(cmd_line, sheli) {

}

void CatCommand::execute() {
    if(this->NumOfArgs==1){
        cout<<"smash error: cat: not enough arguments"<<endl;
    }
    string line;
    for(int i=1;i<this->NumOfArgs;i++){
        ifstream myfile (this->args[i]);
        if(myfile.is_open()){
            while(getline(myfile,line)){
                cout<<line<<endl;
            }
            myfile.close();

        }

    }

}

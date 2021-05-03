#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
class SmallShell;
class Command {
public:
    SmallShell* MyShell;
    char* c_line;
    char** args;
    int NumOfArgs;
    bool isBackground;
    bool Stopped;
    pid_t pid;

    Command(const char* cmd_line, SmallShell* sheli);
    virtual ~Command()=default;
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    int Type;
    PipeCommand(const char* cmd_line,SmallShell* sheli,int type);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    int TypeOfRed;
    explicit RedirectionCommand(const char *cmdLine, SmallShell *sheli, int type);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    char prevdir[1024];
    ChangeDirCommand(const char *cmdLine, SmallShell* shell );
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    explicit GetCurrDirCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

/** ############# MY SHIT #################### **/
class ChangePrompt : public BuiltInCommand {
public:
    ChangePrompt(const char *cmd_line, SmallShell *shell);
    virtual ~ChangePrompt() {}
    void execute() override;
};


class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    QuitCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~QuitCommand() {}
    void execute() override;
};




class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
        int JobId;

        int ProcessID;


    public:
        time_t SecondsElapsed;
        Command* command;
        bool IsStopped;
        bool IsFinished;
        JobEntry(int JobId, Command* command, int processID);
        ~JobEntry()=default;
        void StopJob();
        char* GetJobCommandLine();
       // void PrintJob();
        void FinishedJob();
        int GetJobID();
        int GetProcessID();
        time_t getDuration() const;
        friend std::ostream& operator<< (std::ostream& o,JobEntry const& Job);
    };
    // TODO: Add your data members
    int MaxJobId;
    std::vector<JobEntry>* EntryJobs;
public:
    JobsList();
    ~JobsList();
    void addJob(Command* cmd, pid_t pid, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    //JobEntry * getLastJob(int* lastJobId);
   // JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~KillCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class CatCommand : public BuiltInCommand {
public:
    CatCommand(const char* cmd_line,SmallShell* sheli);
    virtual ~CatCommand() {}
    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members

    SmallShell();
public:
    JobsList* backgroundJobs;
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    char previosdir[1024];
    const char* Prompt;
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};


#endif //SMASH_COMMAND_H_

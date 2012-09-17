class Timer
{
    private:
    //The clock time when the timer started
    int startTicks;
    int intv;
    //The ticks stored when the timer was paused
    int pausedTicks;
    
    //The timer status
    bool paused;
    bool started;
    
    public:
    //Initializes variables
    Timer();
    
    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();
    void SetInterval(int);
    //Gets the timer's time
    int get_ticks();
	int done();
    
    //Checks the status of the timer
    bool is_started();
    bool is_paused();    
};
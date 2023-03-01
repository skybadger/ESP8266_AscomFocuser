/*
 * Focuser high level functions
 */
/* provide a ptr to a variable to use to feedback completion state */

ETSTimer stepTimer;
int FocuserUpdateFlag = 0; //Use this for the Focuser to flag its state. 1 is active

void onStepTimer( void* pArg );

void onStepTimer( void* pArg )
{
  _stepFlag = false;
  //move the Focuser
  Focuser.Update();
}

class Focuser 
{
public:
  //States to move through - when this is moved to a state machine
  enum FocuserStates { FOCUSER_IDLE, FOCUSER_MOVING, FOCUSER_BL_PRE, FOCUSER_BL_POST, FOCUSER_ERROR, FOCUSER_TEMP_COMP, FOCUSER_HALTED };
  static const char* focuserStateCh[] = { "focuser IDLE", "Focuser MOVING", "focuser BACKLASH pre", "focuser ERR", "focuser TEMP COMP" };
  enum FocuserCmds { FOCUSER_HALT, FOCUSER_MOVE, FOCUSER_ABORT, FOCUSER_RDY, FOCUSER_NOTRDY };
  
  static enum FocuserStates focuserState = FocuserStates::FOCUSER_IDLE;
  static enum FocuserStates targetFocuserState = FocuserStates::FOCUSER_IDLE;
  enum FocuserCmds   focuserCmd = FocuserCmds::FOCUSER_NOTRDY;
  
  
  static const int defaultMinLimit = 0;
  static const int defaultMaxLimit = 2<<16-1;
  static const int maxIncrementDefault = 50;
  static const int minIncrementDefault = 1;
  static const int defaultBacklashDistance = 25;
  static const int defaultBacklashDirection = DIRN_CW;
  //These are a bit ambiguous  - they are times per motion interval in microseconds - so a fast speed has a short time that cannot go below 250uS while a slow speed has a big time that shouldnt exceed 20000uS for a stepper.
  //For a stepper we play with microsteps etc to manage achieved slew rates and to keep it in range. 
  //For a DC brushed motor these represent on interval times and they need swapping or inverting. 
  static const int startSpeedDefault = 0;
  static int endMaxSpeedDefault = 6;//will be redefined base on which type and model of motor. 

private: 
  //Indicates whether the focuser is currently moving.
  bool _isMoving = false;

  //Positioning 
  int _maxLim = defaultMaxLimit;
  int _minLim = defaultMinLimit;
  int maxStepIncrement = maxIncrementDefault;
  int minStepIncrement = minIncrementDefault;

  //Maintain the focuser position 
  int _absMode = true;
  int _position = 0;
  int _absPosition = 0; //Absolute position when the focuser last entered relative position mode. Used to keep track of actual position through relative movements
  int _targetDistance = 0;
  int _resolution = 2048;//set according to your motor
  int _microsteps = 1;
  float _degsPerRev = 0.0F;
  int _direction = DIRN_CW;
  int _gear = 1;
  int _positiveDirection = DIRN_CW; //Until changed for some reason. 
 
  //Manage backlash
  //Go to target, wind out in a single direction and then wind back in to target once complete so always approach from a single direction
  //bool reverseDirectionIndicators = false;
  bool _backlashEnabled = false;
  int _backlashDirection = DIRN_CW; //Direction to wind out backlash
  int _backlashDistance = 25; //Distance to use to windout backlash. 

  //State flags
  volatile static bool _focuserEnabledFlag;
  volatile static int _stepFlag;
  volatile static int _t2Flag;
  
  //Callback flags
  int* focuserStateFlag  = nullptr;

  //Manage motion profiles
  /* Variables to track Focuser acceleration and deceleration positions. 
  Starting at start distance of zero from current. we accelerate until we have covered distance[0] in a direction[0] . 
  At that point we cruise for distance[1] in direction [1]
  After that we check to see we have covered the second distance + the first distance in distance[2] before we start decelerating to distance[4]
  Then if backlash is enabled we cruise in the backlash takeup direction into the final position. 
  We calculate these separately for each motion. 
  */
  enum TrapezeSegment { TRAP_IDLE=0, TRAP_ACCEL=1, TRAP_CONSTANT=2, TRAP_DECEL=3, TRAP_HALT=4 };
  enum TrapezeSegment trapezeSegment = TRAP_IDLE;
  struct TrapezeLimit { int distance; int direction; int stepTimestart; int stepTimeEnd; int stepTimeIncr; enum TrapezeSegment segment };
  struct TrapezeLimit _FocuserState[5];
    
  //Sets the device's temperature compensation mode.
  //​Indicates whether the focuser has temperature compensation.
  bool _tempCompAvailable = false;
  
  //In order for tempcomp to work, we need a source of local temperature. 
  char* mqttTemperatureSource = "skybadger/espsen01/htu81/temperature";

  float _temperature;
  bool tempCompEnabled = false;
  const int tempOffsetsLength = 16;
  
  //This should really be a list that can be sorted by termperature and have data points dynamically added/removedand sorted.
  //also needs a means of interpolating and retrieving .  
  struct { float temp; int offset; } TempCompTuple;
  TempCompTuples* tempCompTuples = nullptr;
  
  void addTempDatum( float temp, int position);
  bool sortTempData();
  int getPositionByTemp(int temp );
    
  public:
  Focuser(): _dirPin(1), _stepPin(2), _enPin(0), _stepPosition(0) 
  {
    init();
  };
  
  Focuser( int dirPin, int stepPin, int enablePin, int startingPosition ) : _dirPin(dirPin), _stepPin(stepPin), _enPin(enablePin), _stepPosition(position) 
  {
    init();
  };
  
  bool begin();
  
  //Motor control functions 
  void Enable( int* );
  void Update();
  void Disable( );
  void Halt( bool abortFlag );//abortFlag = 0 for managed halt, 1 for immediate. 
  bool Status(); //0 for idle and 1 for active 
  int readFromEEprom( int startOffset );
  int writeToEeprom();
  
  //Configure Hardware parameters
  bool setPins( int step, int dir, int enable );

    
  //Configure focuser parameters
  bool setLimits( int minLim, int maxLim  );//0 for success and not-zero for failure to set. 
  bool getMinLimit( );//0 for success and not-zero for failure to set. 
  bool getMaxLimit( );//0 for success and not-zero for failure to set. 
  bool setPositiveDirection( enum Directions );
  bool setPosition( int newPos );//0 for success and not-zero for failure to set. 
  bool getPosition( ) { return _position; };
  bool getAbsMode()   { return _absMode; };
  bool setResolution( int stepsPerRev, int gear, int microsteps, int travelPerRev );
  
  //Backlash handling
  void enableBacklash( bool );
  void setupBacklash( int size, int direction );
  bool getBacklashEnabled()  { return _backlashEnabled; };
  int getBacklashSize()      { return_backlashDistance; };
  int getBacklashDirection() { return _backlashDirection; };

  private: 
 
  void updateDirection( bool direction )
  {
    if( direction == DIRN_CCW || direction == DIRN_CW )
      _direction = direction;
    return;
  }
  
  protected:
  void init();
  
  //Motion handling
  bool calcTrajectory( );
  int  calcAccelSteps( startSpeed, finalSpeed );
  
  private:
  ~Focuser();
};

  void Focuser::init( void )
  {
    Motor motor = new Motor( );
    //Setup variables.   
    _trapezeSegment = TRAP_IDLE;
    
    //Setup Timer for Focuser motor step timing
    ets_timer_setfn( &stepTimer, onStepTimer, NULL ); 
  };
  
 bool Focuser::calcTrajectory()
 {
    float maxAccel = 3.0F;
    int halfDistance = 0;
    float accelTime = 0.0F;
    int numAccelSteps = 0;
    
    //Adjust target positions with backlash compensation 
    if( _backlashEnabled )
    {
      distance = _targetPosition - _position;
      if ( _positiveDirection == _backlashDirection ) //We stop early and creep in
      {
        if( distance > 0 ) 
        {
          distance -= _backlashDistance;
        }
        else
        {
          distance += _backlashDistance;
        }       
      }
      else //We go past and come back. 
      {
        if ( distance > 0 ) 
          distance += _backlashDistance;
        else
          distance -= _backlashDistance;        
      }
    }
    
    //Accel 
    halfDistance = ( distance )/2;
    
    //Calculate number of steps to get to cruising speed from cold start with standard accel profile
    //numAccelSteps = numAccel( minSpeed, topSpeed);
    //if( numAccelSteps > halfDistance )
        
    //v2=u2 + 2as
    //s=ut + 1/2 at2
    
    //Setup direction
    if( _positiveDirection == DIRN_CCW ) 
    {
      if( halfDistance > 0 )
        direction  = DIRN_CCW;
      else
        direction  = DIRN_CW;
    }
    else
    {
      if( halfDistance > 0 )
        direction  = DIRN_CW;
      else
        direction  = DIRN_CCW;
    }   
       
    //Stand-ins for now. 
    //Accelerate to constant speed at a specific distance. 
   _FocuserState[0] = { halfDistance; direction; 1000; 250; 25; TRAP_ACCEL };
    //move at constant speed - for a distance
   _FocuserState[1] = { halfDistance; direction; 1000; 250; 25; TRAP_CRUISE };
    //decelerate to stop at target or backlash position.     
   _FocuserState[2] = { halfDistance; direction; 1000; 250; 25; TRAP_DECEL };
    //run in _backlashDirection for _backlashDistance at backlash takeup speed to reach target position.  
   if ( backlashEnabled) 
   {
      _FocuserState[3] = { _backlashDistance; _backlashDirection; 500; 500; 0; TRAP_CRUISE };
   }
   _trapezeSegment = TRAP_IDLE;
 }
  
  /* This function is used to start a motion of the Focuser 
  It runs to completion and reports success or you halt it by calling disableFocuser() 
  */
  void Focuser::Enable( int* monitorPtr )
  {
    //stop/start the timer
    if( _FocuserEnabledFlag )
    {
      DEBUGSL1 ("Enabling Focuser");

      //Setup the move
      //Calculate the trajectory 
      calcTrajectory();
      
      //Setup first phase of motion
         
      //current rate in accel curve indicates period per step required. 
      int period = _FocuserState[_trapezeSegment].initial;
      
      ets_timer_arm_new( &stepTimer, period , 1, 0);
      digitalWrite( ENABLE_PIN, LOW );
      stepFlag = 0;
    }
    else
    {
      DEBUGSL1 ("Disabling Focuser");      
      ets_timer_disarm(&stepTimer);
      digitalWrite( ENABLE_PIN, HIGH );
      _trapezeSegment = TRAP_IDLE;      
    }
  }

  //Used to disable motion at end of profile
  void Focuser::Disable()
  {
     FocuserEnabledFlag = false;
     Halt();   
     //Tell the main loop. 
     *focuserStateFlag = FOCUSER_HALTED;
  }

  //Used to issue rapid halt to Focuser motor. 
  void Focuser::Halt( bool flag )
  {
     //TODO
     //check the numnber of microsteps left to a normal step
     //complete those microsteps to leave the Focuser in a known state on a step detent boundary.  
     //Create emergency decelerate profile 
     
  }
  
  bool Focuser::Status()
  {
     return FocuserEnabledFlag;
  }

  bool Focuser::setLimits( int minLim, int maxLim )
  {
    int status = -1;
    if ( !stepFlagEnabled )
    {
      if ( minLim < maxLim && minLim >= minStepDefault && maxLim <= maxStepDefault ) 
      {
        _minLim = minLim;
        _maxLim = maxLim;
      }
      status = 0;
    }
    return status;
  }

  int Focuser::getLowerLimit( )
  {
    return _minLim;
  }

  int Focuser::getUpperLimit( )
  {
    return _maxLim;
  }

  bool Focuser::setPosition( int newPos )
  {
    int status = true;
    if ( !stepFlagEnabled && newPos >= _minLim && newPos <= _maxLim )
    {
      _stepPosition = newPos;
      status = false;    
    }
    return status;
  }

  bool setResolution( int stepsPerRev, int gear, int microsteps, int degsPerRev )
  {
    int status = 0;
    if ( stepsPerRev > 0 && stepsPerRev <= 400 && gear >=0 && microsteps >= 0 && microsteps <= 256 && degsPerRev >= 0.0 && degsPerRev <= 10.0 ) 
    {
      _degsPreRev  = degsPerRev;
      _stepsPerRevolution  = stepsPerRev; 
      _gear = gear;
      _microsteps = microsteps;    
     _resolution = degsPerRev/( _stepsPerRevolution * _gear * _microsteps );
    }
    else 
      status= -1;
  return status;
  }
  
  static void Focuser::onStep(void)
  {
  	//check targetFocuserState - if still enabled carry on, else abort and stop timer. 
  	if( !FocuserEnabledFlag )
    {
      //decelerate to a halt. 
      ets_timer_disarm(&stepTimer);
            
      //turn off 
      ets_timer_disarm(&stepTimer);
    }
  	
  	if( targetDistance != 0 ) 
  	{
  		//toggle step line
  		digitalWrite(STEP_PIN, LOW);
  		delayMicroseconds(10);
  		digitalWrite(STEP_PIN, HIGH);
  		delayMicroseconds(10);
  		digitalWrite(STEP_PIN, LOW);
  	}
    
    //Book-keeping of position and distance. 
    if ( _stepDirn == DIRN_CW)
    {        
    		_position++;
    		_targetDistance--; 
    }
    else //DIRN_CCW
    {
        _position--;
		    _targetDistance++;
    } 
    stepFlag--;
  	
  	//re-engage timer interrupt unless no more target steps to take ? 
  }

  void Focuser::updateDirection( bool newDirection )
  {
    //Setup DIRN line
    digitalWrite(DIRN_PIN, newDirection );
    delayMicroseconds(10);
  }

  Focuser::saveToEEprom( int offset ) 
  {
  int eepromAddr = offset;
  //Position settings
  EEPROMWriteAnything( eepromAddr, _absMode );
  eepromAddr += sizeof( _absMode );
  DEBUGS1( "Written absMode: ");DEBUGSL1( _absMode );

  EEPROMWriteAnything( eepromAddr, _position );
  eepromAddr += sizeof( _position );
  DEBUGS1( "Written position: ");DEBUGSL1( _position );0
  
  EEPROMWriteAnything( eepromAddr, _minStep );
  eepromAddr += sizeof( _minStep );
  DEBUGS1( "Written minStep: ");DEBUGSL1( _minStep );
  
  EEPROMWriteAnything( eepromAddr, _maxStep );
  eepromAddr += sizeof( _maxStep );
  DEBUGS1( "Written maxStep: ");DEBUGSL1( _maxStep );

  EEPROMWriteAnything( eepromAddr, _minStepIncrement );
  eepromAddr += sizeof( _minStepIncrement );  
  DEBUGS1( F("Written minStepIncrement: "));DEBUGSL1( _minStepIncrement );
  
  EEPROMWriteAnything( eepromAddr, _maxStepIncrement );
  eepromAddr += sizeof( _maxStepIncrement );  
  DEBUGS1( F("Written maxStepIncrement: "));DEBUGSL1( _maxStepIncrement );

  EEPROMWriteAnything( eepromAddr, _distancePerRev );
  eepromAddr += sizeof( _distancePerRev);  
  DEBUGS1( F("Written distancePerRev: "));DEBUGSL1( _distancePerRev );
  
  EEPROMWriteAnything( eepromAddr, _stepsPerRev );
  eepromAddr += sizeof( _stepsPerRev );  
  DEBUGS1( F("Written stepsPerRev: "));DEBUGSL1( _stepsPerRev);
  
  EEPROMWriteAnything( eepromAddr, _microstepsPerStep );
  eepromAddr += sizeof( _microstepsPerStep );  
  DEBUGS1( F("Written microstepsPerStep: "));DEBUGSL1( _microstepsPerStep);
   
  EEPROMWriteAnything( eepromAddr, _tempCompEnabled );
  eepromAddr += sizeof( _tempCompEnabled );  
  DEBUGS1( F("Written tempCompEnabled: "));DEBUGSL1( _tempCompEnabled );

  //Save linked list of termperature compensation settings to eeprom 
  //This should really be a list that can be sorted by temperature and have data points dynamically added/removed. 
  //typedef struct { float temp; float offset };
  //TODO List of tempCompTuples
  for int i=0; i< tempOffsetsLength ; i++ )
  {
    // tempCompTuple[i].temperature = 0.0; 
    // tempCompTuple[i].position = 0.0; 
    ;  
 
  }
  if ( _tempCompTuple != nullptr )
    _tempCompTuple = (TempCompTuple*) calloc( 16, sizeof (TempCompTuple)  );

  int readFromEEprom( int offset )
  {
    int eepromAddr = offset;
    //Position settings 
  EEPROMReadAnything( eepromAddr, _absMode );
  eepromAddr += sizeof( _absMode ); 
  DEBUGS1( F("Read absMode: "));DEBUGSL1( _absMode );
  
  EEPROMReadAnything( eepromAddr, _position );
  eepromAddr += sizeof( _position ); 
  DEBUGS1( F("Read position: "));DEBUGSL1( _position );
    
  EEPROMReadAnything( eepromAddr, _minStep );
  eepromAddr += sizeof( _minStep );   
  DEBUGS1( F("Read minStep: "));DEBUGSL1( _minStep );
    
  EEPROMReadAnything( eepromAddr, _maxStep );
  eepromAddr += sizeof( maxStep );   
  DEBUGS1( F("Read maxStep: "));DEBUGSL1( _maxStep );
  
  EEPROMReadAnything( eepromAddr, _minStepIncrement );
  eepromAddr += sizeof( _minStepIncrement ); 
  DEBUGS1( F("Read minStepIncrement: "));DEBUGSL1( _minStepIncrement );
    
  EEPROMReadAnything( eepromAddr, _maxStepIncrement );
  eepromAddr += sizeof( _maxStepIncrement ); 
  DEBUGS1( F("Read maxStepIncrement: "));DEBUGSL1( _maxStepIncrement );

  EEPROMReadAnything( eepromAddr, _positiveDirection );
  eepromAddr += sizeof( _positiveDirection  ); 
  DEBUGS1( F("Read microstepsPerStep: "));DEBUGSL1( _positiveDirection  );

  EEPROMReadAnything( eepromAddr, _distancePerRev );
  eepromAddr += sizeof( _distancePerRev ); 
  DEBUGS1( F("Read maxStepIncrement: "));DEBUGSL1( _distancePerRev );
  
  EEPROMReadAnything( eepromAddr, _stepsPerRev );
  eepromAddr += sizeof( _stepsPerRev ); 
  DEBUGS1( F("Read stepsPerRev: "));DEBUGSL1( _stepsPerRev );

  EEPROMReadAnything( eepromAddr, _microsteps );
  eepromAddr += sizeof( _microsteps ); 
  DEBUGS1( F("Read microstepsPerStep: "));DEBUGSL1( _microsteps );
  
  EEPROMReadAnything( eepromAddr, _gear );
  eepromAddr += sizeof( _gear  ); 
  DEBUGS1( F("Read microstepsPerStep: "));DEBUGSL1( _gear  );

  EEPROMReadAnything( eepromAddr, _degsPerRev );
  eepromAddr += sizeof( _degsPerRev ); 
  DEBUGS1( F("Read microstepsPerStep: "));DEBUGSL1( _degsPerRev );


  //Maintain the focuser position 
//  int _absPosition = 0; //Absolute position when the focuser last entered relative position mode. Used to keep track of actual position through relative movements
//  int _resolution = 2048;//set according to your motor
 
  //Backlash
  EEPROMReadAnything( eepromAddr, _backlashEnabled );
  eepromAddr += sizeof( _backlashEnabled  ); 
  DEBUGS1( F("Read maxStepIncrement: "));DEBUGSL1( _backlashEnabled  );
  
  EEPROMReadAnything( eepromAddr, _backlashDirection );
  eepromAddr += sizeof( _backlashDirection ); 
  DEBUGS1( F("Read stepsPerRev: "));DEBUGSL1( _backlashDirection );

  EEPROMReadAnything( eepromAddr, _backlashDistance );
  eepromAddr += sizeof( _backlashDistance ); 
  DEBUGS1( F("Read microstepsPerStep: "));DEBUGSL1( _backlashDistance );

  //Temp compensation  
  EEPROMReadAnything( eepromAddr, _tempCompEnabled );
  eepromAddr += sizeof( _tempCompEnabled ); 
  DEBUGS1( F("Read tempCompEnabled: "));DEBUGSL1( tempCompEnabled );

  //TODO List of tempCompTuples
  if ( _tempCompTuple != nullptr )
    _tempCompTuple = (TempCompTuple*) calloc( 16, sizeof (TempCompTuple)  );
  for int i=0; i< tempOffsetsLength ; i++ )
  {
    // tempCompTuple[i].temperature = 0.0; 
    // tempCompTuple[i].position = 0.0; 
    ;
  }
  }

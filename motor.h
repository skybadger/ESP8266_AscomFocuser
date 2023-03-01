class Motor {

  public: 

  //Basic Focuser info - update based on your Focuser
  enum EnableModes { ENABLE_NONE, ENABLE_LOW, ENABLE_HIGH };
  
  Motor::Motor();
  void setupPins();

  void step();
  void setupMotor( int stepsPerRev, int microSteps );
  void setupMotor( int dirPin, int stepPin, int enablePin, int enableMode ) : _dirPin(dirPin), _stepPin(stepPin), _enPin(enablePin), _stepPosition(position)
  {
    init();
  }
   
  
  private: 
  //Hardware Control motor pins
  int _stepPin = 0;
  int _dirPin = 0;
  int _enPin = 0;
  int enableMode = EnableModes::ENABLE_NONE;
  
  protected: 
  void init();
   
}

void Motor::init(void) 
{
    //Setup hardware
    pinMode(DIRN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite( DIRN_PIN, DIRN_CW );
    digitalWrite( STEP_PIN, LOW);
    digitalWrite( ENABLE_PIN, HIGH); //Active low.
}

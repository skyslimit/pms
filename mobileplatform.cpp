#include "mobileplatform.h"

MobilePlatform::MobilePlatform(QObject *parent) :
    QObject(parent)
  ,MotorLeft(s_nMotorLeftPin1,s_nMotorLeftPin2,s_nMotorLeftPinPWM)
  ,MotorRight(s_nMotorRightPin1,s_nMotorRightPin2,s_nMotorRightPinPWM)
  ,LineSensorLeft(s_nLineSensorLeft)
  ,LineSensorRight(s_nLineSensorRight)
{
  bFollowing = false;
  m_nLastCmdSpeedLeft = 40;
  m_nLastCmdSpeedRight = 40;
  m_fReductionFactor = 0.3;

}
void MobilePlatform::initializeGPIO()
{

  if(wiringPiSetup() == -1){
      emit logMsg("GPIO Pin Setup failed");
      emit logMsg("Motors and Sensors not initialized");
      return;
  }

  if(MotorLeft.initPins())
      emit logMsg("Left motor initialized");
  else
      emit logMsg("[ERROR]: Left motor not initialized");

  if(MotorRight.initPins())
      emit logMsg("Right motor initialized");
  else
      emit logMsg("[ERROR]: Right motor not initialized");

  if(LineSensorLeft.initPin())
      emit logMsg("Left line sensor initialized");
  else
      emit logMsg("[ERROR]: Left line sensor not initialized");

  if(LineSensorRight.initPin())
      emit logMsg("Right line sensor initialized");
  else
      emit logMsg("[ERROR]: Right line sensor not initialized");
}

void MobilePlatform::setReductionFactor(double factor)
{
    m_fReductionFactor = factor;
    std::stringstream ssOut;
    ssOut << "Reduction factor set to: "<< m_fReductionFactor;
    emit logMsg(QString::fromStdString(ssOut.str()));
}

void MobilePlatform::moveForward()
{
  MotorLeft.forward();
  MotorRight.forward();
  emit logMsg("Moving forward");
}

void MobilePlatform::moveBackward()
{
  MotorLeft.backward();
  MotorRight.backward();
  emit logMsg("Moving backward");
}

void MobilePlatform::turnLeft()
{
  MotorLeft.stop();
  MotorRight.forward();
  emit logMsg("Turning left");
}

void MobilePlatform::turnRight()
{
  MotorLeft.forward();
  MotorRight.stop();
  emit logMsg("Turning right");
}

void MobilePlatform::stopMotion()
{
  MotorLeft.stop();
  MotorRight.stop();
  bFollowing = false;
  emit logMsg("Stopping");
}

void MobilePlatform::startLineFollowing()
{//Call Function to start line following, only if it is not started already
  if(!bFollowing)
  {
    bFollowing = true;
    emit followingTriggered(true);
    followLine();
  }
  else emit followingTriggered(false);
}

int MobilePlatform::setPWM(int spnbxValue)
{
  m_nLastCmdSpeedLeft = MotorLeft.setPWM(spnbxValue);
  m_nLastCmdSpeedRight = MotorRight.setPWM(spnbxValue * 0.9);//Reduce speed of right motor to have straight movement

  std::stringstream ssOut;
  ssOut << "Set PWM: Left Motor: "<< m_nLastCmdSpeedLeft << ", Right Motor: " << m_nLastCmdSpeedRight;
  emit logMsg(QString::fromStdString(ssOut.str()));
  return m_nLastCmdSpeedLeft;
}

void MobilePlatform::followLine()
{
  int m_nSpeedLeft;
  int m_nSpeedRight;

  while(bFollowing)
  {
    bool bLeftSensorOnLine = LineSensorLeft.isOverBlack();
    bool bRightSensorOnLine = LineSensorRight.isOverBlack();
    if (!bLeftSensorOnLine && bRightSensorOnLine){
        m_nSpeedLeft = m_nLastCmdSpeedLeft;
        m_nSpeedRight = m_nLastCmdSpeedRight;
        emit logMsg("Going straight!");
    }
    else if (bLeftSensorOnLine && bRightSensorOnLine){
        m_nSpeedLeft = m_nLastCmdSpeedLeft *  m_fReductionFactor;
        m_nSpeedRight = m_nLastCmdSpeedRight;
        emit logMsg("Going left!");
    }
    else if (bLeftSensorOnLine && !bRightSensorOnLine){
        m_nSpeedLeft = m_nLastCmdSpeedLeft *  m_fReductionFactor;
        m_nSpeedRight = m_nLastCmdSpeedRight;
        emit logMsg("Going left!");
    }
    else if (!bLeftSensorOnLine && !bRightSensorOnLine){
        m_nSpeedLeft = m_nLastCmdSpeedLeft;
        m_nSpeedRight = m_nLastCmdSpeedRight *  m_fReductionFactor;
        emit logMsg("Going right!");
    }
    else{
      emit logMsg("[ERROR]: Line sensor output not catched");
        m_nSpeedLeft = 0;
        m_nSpeedRight = 0;
    }
    MotorLeft.setPWM(m_nSpeedLeft);
    MotorRight.setPWM(m_nSpeedRight);
    MotorLeft.forward();
    MotorRight.forward();

    qApp->processEvents();
  }
  MotorLeft.setPWM(m_nLastCmdSpeedLeft);
  MotorRight.setPWM(m_nLastCmdSpeedRight);
}

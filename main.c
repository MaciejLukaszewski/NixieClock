/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

MultiButton B0,B1;
Clk Clock;
LightUpTube NixieToSet;

volatile uint8_t timerFlag = 0;
volatile uint8_t alarmFlag = 0;
volatile uint8_t ADC_val = 0;		//ADC value to control luminance
volatile uint8_t flagUsart = 0;

volatile uint32_t currentTicksB0;
volatile uint32_t previousTicksB0;
volatile uint32_t currentTicksB1;
volatile uint32_t previousTicksB1;

uint32_t Luminosity1 = 500;
uint32_t Luminosity2 = 500;
uint32_t Luminosity3 = 500;
uint32_t Luminosity4 = 500;

DisplayTime TimeToDisp;
DisplayTime SettingTim;

uint32_t HoursUsart;
uint32_t MinutesUsart;
uint8_t Received[5];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM14_Init(void);
static void MX_RTC_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
/*
 * Takes hours, minutes and seconds as decimal value and sets RTC time
 */
void set_time(uint8_t hours, uint8_t minutes, uint8_t seconds){
	RTC_TimeTypeDef sTime = {0};

	sTime.Hours = hours;
	sTime.Minutes = minutes;
	sTime.Seconds = seconds;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
	  Error_Handler();
	}
}
/*
*Set Alarm time
*/
void set_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds){
	RTC_AlarmTypeDef sAlarm = {0};

	sAlarm.AlarmTime.Hours = hours;
	sAlarm.AlarmTime.Minutes = minutes;
	sAlarm.AlarmTime.Seconds = seconds;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 0x1;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}
/*
 * Takes pointer to the timer and the compare time value
 * If timer exceeds the time returns Enable
 */
uint8_t checkTimer( uint32_t *timer, uint32_t time){
	uint8_t check  = RESET;											//init variable
	check = ((HAL_GetTick() - *timer) > time ) ? ENABLE : DISABLE;	// check if specific ticks passed
	return check;													//return enable if 'time' count was exceeded
}
/*
 * Init of button structure - takes pointer to the structure
 */
void ButtonInit( MultiButton *button ){
	button->flag = false;
	button->buttonState = NotPressed;
	button->buttonTimerEnable = RESET;
	button->buttonHoldCounter = 0;
	button->timer = 0;
}

void TimeInit( TimeHolder *time){
	time->Hours = 0;
	time->Minutes = 0;
}

/*
 * Init of Clock structure - takes pointer to the structure
 */
void ClockInit( Clk *Clock ){
	Clock->Alarm = OFF;
	Clock->ClockState = Time;
	Clock->DispMode = ContinuouseMode;
	Clock->Tube = All;
	Clock->NextNixie = Nixie1;
	TimeInit(&(Clock->ClockTime));
	TimeInit(&(Clock->AlarmTime));
}

void WriteDigit( uint8_t * number ){
	HAL_GPIO_WritePin(NixieDigitA_GPIO_Port, NixieDigitA_Pin, (*number) & 0x01 );
	HAL_GPIO_WritePin(NixieDigitB_GPIO_Port, NixieDigitB_Pin, (((*number)>> 1) & 0x01) );
	HAL_GPIO_WritePin(NixieDigitC_GPIO_Port, NixieDigitC_Pin, (((*number)>> 2) & 0x01) );
	HAL_GPIO_WritePin(NixieDigitD_GPIO_Port, NixieDigitD_Pin, (((*number)>> 3) & 0x01) );
}

/*
 * Ligh up Tubes with correct Luminosity
 */
void NixieLightUp(){


	if( Clock.ClockState == TimeSet || Clock.ClockState == AlarmSet){
		switch(NixieToSet){
			case Nixie1:
				Luminosity1 = 500;
				Luminosity2 = 100;
				Luminosity3 = 100;
				Luminosity4 = 100;
				break;
			case Nixie2:
				Luminosity1 = 100;
				Luminosity2 = 500;
				Luminosity3 = 100;
				Luminosity4 = 100;
				break;
			case Nixie3:
				Luminosity1 = 100;
				Luminosity2 = 100;
				Luminosity3 = 500;
				Luminosity4 = 100;
				break;
			case Nixie4:
				Luminosity1 = 100;
				Luminosity2 = 100;
				Luminosity3 = 100;
				Luminosity4 = 500;
				break;
			default:
				break;
		}
	}
	else{
		Luminosity1 = ADC_val * 8 + 64 - ADC_val;
		Luminosity2 = Luminosity1;
		Luminosity3 = Luminosity1;
		Luminosity4 = Luminosity1;
	}
	switch (Clock.NextNixie){
		case Nixie1 :			//turn on 1st Clock.NextNixie and turn off 4th one
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Luminosity1);

			if( Clock.Tube == All || Clock.Tube == Nixie1 ){
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
				for(int i =0;i<190;i++);
				WriteDigit(&TimeToDisp.HoursDecimal);
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
				Clock.NextNixie = Nixie2;
			}
			else{
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
				Clock.NextNixie = Nixie2;
			}
			break;
		case Nixie2 :			//turn on 2nd Clock.NextNixie and turn off 1st one
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, Luminosity2);

			if( Clock.Tube == All || Clock.Tube == Nixie2 ){
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
				for(int i =0;i<190;i++);
				WriteDigit(&TimeToDisp.HoursUnity);
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
				Clock.NextNixie = Nixie3;
			}
			else{
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
				Clock.NextNixie = Nixie3;
			}
			break;
		case Nixie3 :			//turn on 3rd Clock.NextNixie and turn off 2nd one
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Luminosity3);

			if( Clock.Tube == All || Clock.Tube == Nixie3 ){
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
				for(int i =0;i<190;i++);
				WriteDigit(&TimeToDisp.MinutesDecimal);
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
				Clock.NextNixie = Nixie4;
			}
			else{
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
				Clock.NextNixie = Nixie4;
			}
			break;
		case Nixie4 :			//turn on 4th Clock.NextNixie and turn off 3rd one
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, Luminosity4);

			if( Clock.Tube == All || Clock.Tube == Nixie4 ){
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
				for(int i =0;i<190;i++);
				WriteDigit(&TimeToDisp.MinutesUnity);
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
				Clock.NextNixie = Nixie1;
			}
			else{
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
				Clock.NextNixie = Nixie1;
			}
			break;
		default:
			break;
	}
	HAL_ADC_Start_IT(&hadc);
}
/*
 * Change time from TimeHolder type to TimeCount type
 */
void TimeToNixie( TimeHolder *time, DisplayTime *TimeCount ){
	TimeCount->HoursUnity = time->Hours % 10;									// extract decimal value for tube 2
	TimeCount->HoursDecimal = (time->Hours - TimeCount->HoursUnity) / 10;  			// extract decimal value for tube 1
	TimeCount->MinutesUnity = time->Minutes % 10;            					// extract decimal value for tube 4
	TimeCount->MinutesDecimal = (time->Minutes - TimeCount->MinutesUnity) / 10;		// extract decimal value for tube 3
}
/*
 * Change time frm TimeCount type to TimeHolder type
 */
void NixieToTime( DisplayTime *TimeCont ,TimeHolder *time ){
	time->Hours = TimeCont->HoursDecimal*10 + TimeCont->HoursUnity;
	time->Minutes = TimeCont->MinutesDecimal*10 + TimeCont->MinutesUnity;
}

void StaticVarInit(){
	// time holding structure init
	TimeToDisp.HoursDecimal = 0;
	TimeToDisp.HoursUnity = 0;
	TimeToDisp.MinutesDecimal = 0;
	TimeToDisp.MinutesUnity = 0;
	// time holding structure init
	SettingTim.HoursDecimal = 0;
	SettingTim.HoursUnity = 0;
	SettingTim.MinutesDecimal = 0;
	SettingTim.MinutesUnity = 0;
	// variable responsible for first tube to set
	NixieToSet = Nixie1;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


	/*
	 * Initialization of structures and variables
	 */
	ButtonInit(&B0);
	ButtonInit(&B1);
	ClockInit(&Clock);
	StaticVarInit();
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */


  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_ADC_Init();
  MX_TIM14_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /*
   * Start all timers, ADC, and set default clock time
   */
  HAL_UART_Receive_IT(&huart1, Received, 5);
  HAL_ADC_Start_IT(&hadc);	//start ADC conversions
  HAL_TIM_Base_Start_IT(&htim14);
  HAL_TIM_Base_Start(&htim1);
  set_time(0, 0, 0);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
  while (1){

	  if( flagUsart ){
		  flagUsart = 0;
		  if( Received[0] == 84){
			  HoursUsart = ((int)Received[1] - 48)*10 + ((int)Received[2] - 48);
			  MinutesUsart = ((int)Received[3] - 48)*10 + ((int)Received[4] - 48);
			  if( HoursUsart > 23 || HoursUsart < 0){
				  HAL_UART_Receive_IT(&huart1, Received, 5);
			  }
			  else{
				  Clock.ClockTime.Hours = HoursUsart;
				  Clock.ClockTime.Minutes = MinutesUsart;
				  set_time(HoursUsart, MinutesUsart,0);
				  HAL_UART_Receive_IT(&huart1, Received, 5);
			  }
		  }
	  }


	  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);			//get time from RTC as decimal
	  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);			// get date - necessary to start RTC again
	  Clock.ClockTime.Hours = sTime.Hours;						// get the hours
	  Clock.ClockTime.Minutes = sTime.Minutes;					// get the minutes

	  switch(Clock.ClockState){
	  	  case Time:
	  		  if( Clock.Tube == All){
				  HAL_GPIO_WritePin(GPIOB, BLUE_TIME_MODE_Pin, GPIO_PIN_SET);
				  HAL_GPIO_WritePin(GPIOB, GREEN_ALARM_MODE_Pin, GPIO_PIN_RESET);
				  HAL_GPIO_WritePin(GPIOB, RED_ALARM_OC_Pin, GPIO_PIN_RESET);
	  		  }
	  		  else{
	  			HAL_GPIO_WritePin(GPIOB, BLUE_TIME_MODE_Pin, GPIO_PIN_RESET);
	  			HAL_GPIO_WritePin(GPIOB, GREEN_ALARM_MODE_Pin, GPIO_PIN_RESET);
	  			HAL_GPIO_WritePin(GPIOB, RED_ALARM_OC_Pin, GPIO_PIN_RESET);
	  		  }
	  		  TimeToNixie( &Clock.ClockTime, &TimeToDisp);					//write time to disp variable
	  		  break;
	  	  case Alarm:
	  		  if( Clock.Tube == All){
				  HAL_GPIO_WritePin(GPIOB, BLUE_TIME_MODE_Pin, GPIO_PIN_RESET);
				  if(alarmFlag){
					  HAL_GPIO_WritePin(GPIOB, GREEN_ALARM_MODE_Pin, GPIO_PIN_RESET);
					  HAL_GPIO_WritePin(GPIOB, RED_ALARM_OC_Pin, GPIO_PIN_SET);
				  }
				  else if(Clock.Alarm == ON ){
					  HAL_GPIO_WritePin(GPIOB, GREEN_ALARM_MODE_Pin, GPIO_PIN_SET);
					  HAL_GPIO_WritePin(GPIOB, RED_ALARM_OC_Pin, GPIO_PIN_RESET);
				  }
				  else{
					  HAL_GPIO_WritePin(GPIOB, GREEN_ALARM_MODE_Pin, GPIO_PIN_RESET);
					  HAL_GPIO_WritePin(GPIOB, RED_ALARM_OC_Pin, GPIO_PIN_SET);
				  }
	  		  }
	  		  else{
	  			  HAL_GPIO_WritePin(GPIOB, BLUE_TIME_MODE_Pin, GPIO_PIN_RESET);
	  			  HAL_GPIO_WritePin(GPIOB, GREEN_ALARM_MODE_Pin, GPIO_PIN_RESET);
	  			  HAL_GPIO_WritePin(GPIOB, RED_ALARM_OC_Pin, GPIO_PIN_RESET);
	  		  }
	  		  TimeToNixie( &Clock.AlarmTime, &TimeToDisp);					//write time to disp variable
	  		  break;
	  	  case TimeSet:
	  		  TimeToDisp = SettingTim;
	  		  break;
	  	  case AlarmSet:
	  		  TimeToDisp = SettingTim;
	  		  break;
	  }


	  if(timerFlag){
		  timerFlag = 0;
		  NixieLightUp();
	  }

	  if(alarmFlag){
		  HAL_GPIO_WritePin(GPIOA, ALARM_Pin, GPIO_PIN_SET);
	  }


	  if(B1.flag){
		  B1.flag = false;
		  if( Clock.ClockState == Time ){
			  switch(Clock.DispMode){
				  case MoveMode:
					Clock.DispMode = ContinuouseMode;
					break;
				  case ContinuouseMode:
					Clock.DispMode = MoveMode;
					break;
				  default:
					  break;
			  }
		  }
		  //add another functionalities of button based on the ClockState
		  else if(Clock.ClockState == Alarm ){				//if display switch to alarm
			  if(Clock.Alarm == ON){
				  Clock.Alarm = OFF;//alarm on/off
				  if( alarmFlag ){
					  alarmFlag = 0;
					  B0.flag = false;
					  HAL_GPIO_WritePin(GPIOA, ALARM_Pin, GPIO_PIN_RESET);
					  Clock.ClockState = Time;
					  //alarm off
				  }
			  }
			  else if( Clock.Alarm == OFF){
				  Clock.Alarm = ON;
			  }
		  }
		  else if(Clock.ClockState == TimeSet ){		//if clock time set
			  switch(NixieToSet){
			  	  case Nixie1:	//nixie value += 1
			  		  if( SettingTim.HoursDecimal < 2 ){
			  			  SettingTim.HoursDecimal += 1;
			  			  if(SettingTim.HoursUnity > 4 && SettingTim.HoursDecimal >= 1){		//dont exceed 24 format hour
			  				SettingTim.HoursUnity = 0;
			  			  }
			  		  }
			  		  else{
			  			  SettingTim.HoursDecimal = 0;
			  		  }
			  		  break;
			  	  case Nixie2:
			  		if(SettingTim.HoursDecimal < 2){  				//dont exceed 24 format hour
						if( SettingTim.HoursUnity < 9 ){
							SettingTim.HoursUnity += 1;
						}
						else{
							SettingTim.HoursUnity = 0;
						}
			  		}
			  		else{											//dont exceed 24 format hour
			  			if( SettingTim.HoursUnity < 3 ){
			  				SettingTim.HoursUnity += 1;
			  			}
			  			else{
			  				SettingTim.HoursUnity = 0;
			  			}
			  		}
			  		break;
			  	  case Nixie3:
			  		if( SettingTim.MinutesDecimal < 5 ){
			  			SettingTim.MinutesDecimal += 1;
			  		}
			  		else{
			  			SettingTim.MinutesDecimal = 0;
			  		}
				  	  break;
			  	  case Nixie4:
			  		if( SettingTim.MinutesUnity < 9 ){
			  			SettingTim.MinutesUnity += 1;
			  		}
			  		else{
			  			SettingTim.MinutesUnity = 0;
			  		}
			  		  break;
			  	  default:
			  		break;
			  }
		  }
		  else if(Clock.ClockState == AlarmSet ){		//if clock alarm set
			  switch(NixieToSet){
			  	  case Nixie1:	//nixie value += 1
					  if( SettingTim.HoursDecimal < 2 ){
						  SettingTim.HoursDecimal += 1;
						  if(SettingTim.HoursUnity > 4 && SettingTim.HoursDecimal >= 1){		//dont exceed 24 format hour
							  SettingTim.HoursUnity = 0;
						  }
					  }
					  else{
						  SettingTim.HoursDecimal = 0;
					  }
					  break;
				  case Nixie2:
					  if(SettingTim.HoursDecimal < 2){  				//dont exceed 24 format hour
						  if( SettingTim.HoursUnity < 9 ){
							  SettingTim.HoursUnity += 1;
						  }
						  else{
							  SettingTim.HoursUnity = 0;
						  }
					  }
					  else{											//dont exceed 24 format hour
						  if( SettingTim.HoursUnity < 3 ){
							  SettingTim.HoursUnity += 1;
						  }
						  else{
							  SettingTim.HoursUnity = 0;
						  }
					  }
					  break;
				  case Nixie3:
					  if( SettingTim.MinutesDecimal < 5 ){
						  SettingTim.MinutesDecimal += 1;
					  }
					  else{
						  SettingTim.MinutesDecimal = 0;
					  }
					  break;
				  case Nixie4:
					  if( SettingTim.MinutesUnity < 9 ){
						  SettingTim.MinutesUnity += 1;
					  }
					  else{
						  SettingTim.MinutesUnity = 0;
					  }
					  break;
				  default:
					  break;
			  }
		  }
	  }


	  if(B0.flag && !alarmFlag){				//if interrupt occurs on this button
		  B0.flag = false;						//clear interrupt button flag
	  	  if(!B0.buttonTimerEnable){			//if timer is not enabled -----> enable
	  		  B0.buttonTimerEnable = SET;		//enable timer
	  		  B0.buttonState = SingleClick;		//state - single click
	  		  B0.timer = HAL_GetTick();			//get tick
	  	  }
	  	  else{									//if timer was enabled before end of operation -> second interrupt occurs -> double click
	  		  B0.buttonState = DoubleClick;
	  	  }
	  }
	  if(checkTimer(&B0.timer, 10 * B0.buttonHoldCounter) && B0.buttonTimerEnable){		//check every 10 ticks and if button timer is enabled
		  if(!HAL_GPIO_ReadPin(B0_GPIO_Port, B0_Pin)){									//if still holded
			  ++B0.buttonHoldCounter;													//increment counter
		  }
		  B0.buttonState = (B0.buttonHoldCounter >= 60) ? Hold : B0.buttonState;		// if button counter >= 60 state == hold else previous state
	  }

	  if(checkTimer(&B0.timer, 600) && B0.buttonTimerEnable){							//check if 600 ticks passed and if timer enabled
		  switch(B0.buttonState){														//decide what to do based on state of button
	  	  	  case SingleClick:													//if single click
	  	  		  switch(Clock.ClockState){											//choose button fuction based on clock state
	  	  		  	  case Time:													//if TIME change to ALARM and vice versa
	  	  		  		  Clock.ClockState = Alarm;
	  	  		  		  Clock.DispMode = ContinuouseMode;
	  	  		  		  break;
	  	  		  	  case Alarm:
	  	  		  		  Clock.ClockState = Time;
	  	  		  		  break;
	  	  		  	  case TimeSet:													// if time set - switch to next nixie to set in round cycle
	  	  		  		  if( NixieToSet == Nixie4){
	  	  		  			  NixieToSet = Nixie1;
	  	  		  		  }
	  	  		  		  else{
	  	  		  			  NixieToSet += 1;
	  	  		  		  }
	  	  		  		  break;
	  	  		  	  case AlarmSet:												// if alarm set - switch to next nixie to set in round cycle
	  	  		  		  if( NixieToSet == Nixie4){
	  	  		  			  NixieToSet = Nixie1;
	  	  		  		  }
	  	  		  		  else{
	  	  		  			  NixieToSet += 1;
	  	  		  		  }
	  	  		  		  break;
	  	  		  }
	  	  		  break;
	  		  case DoubleClick:
	  			switch(Clock.ClockState){
					case Time:
						break;
					case Alarm:
						break;
					case TimeSet:
						break;
					case AlarmSet:
						break;
	  			}
	  			  break;
	  		  case Hold:
	  			switch(Clock.ClockState){
					case Time:
						Clock.ClockState = TimeSet;
						NixieToSet = Nixie1;
						TimeToNixie( &Clock.ClockTime, &SettingTim); 	//write to variable actual time
						break;
					case Alarm:
						Clock.ClockState = AlarmSet;
						NixieToSet = Nixie1;
						TimeToNixie( &Clock.AlarmTime, &SettingTim); 	//write to variable actual alarm
						break;
					case TimeSet:
						Clock.ClockState = Time;
						NixieToTime( &SettingTim, &Clock.ClockTime);
						set_time(Clock.ClockTime.Hours, Clock.ClockTime.Minutes, 0); //write to time REGISTER set time
						break;
					case AlarmSet:
						Clock.ClockState = Alarm;
						NixieToTime(&SettingTim, &Clock.AlarmTime);
						set_alarm(Clock.AlarmTime.Hours, Clock.AlarmTime.Minutes, 0); //write to alarm REGISTER set alarm
						break;
	  			}
	  			break;
	  		  default:
	  			  break;
		  }
		  B0.buttonTimerEnable = RESET;													//disable button timer
		  B0.buttonHoldCounter = RESET;													//reset button counter
	  }

	  if( Clock.DispMode == ContinuouseMode){
		  if( Clock.Tube != All ){			//if exit MoveMode and Tube == None
			  Clock.Tube = All;
		  }
	  }
	  else if (Clock.DispMode == MoveMode){	//MoveMode
		  if(!HAL_GPIO_ReadPin(MoveSensor_GPIO_Port, MoveSensor_Pin)){
			  Clock.Tube = None;
		  }
		  else{
			  Clock.Tube = All;
		  }
	  }
  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_6B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 306;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 80-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 500-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 800-1;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 50;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NixieDigitA_Pin|NixieDigitC_Pin|NixieDigitD_Pin|ALARM_Pin
                          |NixieDigitB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BLUE_TIME_MODE_Pin|GREEN_ALARM_MODE_Pin|RED_ALARM_OC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NixieDigitA_Pin NixieDigitC_Pin NixieDigitD_Pin ALARM_Pin
                           NixieDigitB_Pin */
  GPIO_InitStruct.Pin = NixieDigitA_Pin|NixieDigitC_Pin|NixieDigitD_Pin|ALARM_Pin
                          |NixieDigitB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : B0_Pin */
  GPIO_InitStruct.Pin = B0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(B0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MoveSensor_Pin */
  GPIO_InitStruct.Pin = MoveSensor_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MoveSensor_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BLUE_TIME_MODE_Pin GREEN_ALARM_MODE_Pin RED_ALARM_OC_Pin */
  GPIO_InitStruct.Pin = BLUE_TIME_MODE_Pin|GREEN_ALARM_MODE_Pin|RED_ALARM_OC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if( huart == &huart1){
		flagUsart = 1;
	}
}

/*
 * ADC Callback - write new value to variable
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* h){
	if( h == &hadc ){
		ADC_val = HAL_ADC_GetValue(&hadc);
	}
}

/*
 * TIM14 Callback - NextNixie tube selection - control of TIM1
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if( htim == &htim14 ){
		timerFlag = 1;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == B0_Pin ){
		currentTicksB0 = HAL_GetTick();					//for debounce
		if((currentTicksB0 - previousTicksB0) > 200){
			previousTicksB0 = currentTicksB0;				//for eliminating debounce
			B0.flag = true;
		}
	}

	if(GPIO_Pin == B1_Pin ){
		currentTicksB1 = HAL_GetTick();					//for debounce
		if((currentTicksB1 - previousTicksB1) > 200){
			previousTicksB1 = currentTicksB1;				//for eliminating debounce
			B1.flag = true;
		}
	}
}


void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
	//alarm callback
	if(Clock.Alarm == ON){
		Clock.ClockState = Alarm;			//change state to alarm ( display it and get functionallity of button B0 as on of alarm
		alarmFlag = 1;
	}
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


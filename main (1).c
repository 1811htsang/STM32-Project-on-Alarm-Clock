/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "i2c-lcd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stdbool.h>
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
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define DS3231_ADDRESS 0xD0

// Convert normal decimal numbers to binary coded decimal
uint8_t decToBcd(int val) {
	return (uint8_t) ((val / 10 * 16) + (val % 10));
}
// Convert binary coded decimal to normal decimal numbers
int bcdToDec(uint8_t val) {
	return (int) ((val / 16 * 10) + (val % 16));
}

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
} TIME;

TIME time;

// function to set time

void Set_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom,
		uint8_t month, uint8_t year) {
	uint8_t set_time[7];
	set_time[0] = decToBcd(sec);
	set_time[1] = decToBcd(min);
	set_time[2] = decToBcd(hour);
	set_time[3] = decToBcd(dow);
	set_time[4] = decToBcd(dom);
	set_time[5] = decToBcd(month);
	set_time[6] = decToBcd(year);

	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x00, 1, set_time, 7, 1000);
}

void Get_Time(void) {
	uint8_t get_time[7];
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, 1, get_time, 7, 1000);
	time.seconds = bcdToDec(get_time[0]);
	time.minutes = bcdToDec(get_time[1]);
	time.hour = bcdToDec(get_time[2]);
	time.dayofweek = bcdToDec(get_time[3]);
	time.dayofmonth = bcdToDec(get_time[4]);
	time.month = bcdToDec(get_time[5]);
	time.year = bcdToDec(get_time[6]);
}

float Get_Temp(void) {
	uint8_t temp[2];

	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x11, 1, temp, 2, 1000);
	return ((temp[0]) + (temp[1] >> 6) / 4.0);
}

void force_temp_conv(void) {
	uint8_t status = 0;
	uint8_t control = 0;
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x0F, 1, &status, 1, 100); // read status register
	if (!(status & 0x04)) {
		HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x0E, 1, &control, 1, 100); // read control register
		HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x0E, 1,
				(uint8_t*) (control | (0x20)), 1, 100);
	}
}
float TEMP;
char buffer[15];

int cur = 6;
int alarm_num[10];
int num_i = 0;
//string num_c[0];
int status_button_1;
int status_button_2;
int status_button_3;
int status_button_return;
int status_button_right;
int status_button_reset;
int status_button_number;
int status_button_stop;
bool check_time(int h, int m, int s) {
	if (time.hour == h && time.minutes == m && time.seconds == s)
		return true;
	else
		return false;
}

void update_Time() {
	Get_Time();

	sprintf(buffer, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);
	lcd_put_cur(0, 0);
	lcd_send_string(buffer);

	sprintf(buffer, "%02d-%02d-20%02d", time.dayofmonth, time.month, time.year);
	lcd_put_cur(1, 0);
	lcd_send_string(buffer);

	force_temp_conv();

	TEMP = Get_Temp();

	lcd_put_cur(0, 10);

	sprintf(buffer, "%f", TEMP);

	lcd_send_string(buffer);
}

void button_menu() {
	lcd_put_cur(1, 13);
	lcd_send_string("123");
}

void first_menu() {
	lcd_put_cur(0, 0);
	lcd_send_string("1");
	lcd_put_cur(0, 2);
	lcd_send_string("normal alarm");
	lcd_put_cur(1, 0);
	lcd_send_string("time:");
	lcd_put_cur(1, 6);
	lcd_send_string("00:00:00");
	lcd_put_cur(1, 15);
	lcd_send_string("y");
	lcd_put_cur(1, 14);
	lcd_send_data(0x7F);
	lcd_put_cur(1, 6);
}

void second_menu() {
	lcd_put_cur(0, 0);
	lcd_send_string("2");
	lcd_put_cur(0, 2);
	lcd_send_string("count time");
	lcd_put_cur(1, 0);
	lcd_send_string("time:");
	lcd_put_cur(1, 6);
	lcd_send_string("00:00:00");
	lcd_put_cur(1, 15);
	lcd_send_string("y");
	lcd_put_cur(1, 14);
	lcd_send_data(0x7F);
	lcd_put_cur(1, 6);
}

void third_menu() {
	lcd_put_cur(0, 0);
	lcd_send_string("3");
	lcd_put_cur(0, 2);
	lcd_send_string("alarm w time");
	lcd_put_cur(1, 0);
	lcd_send_string("time:");
	lcd_put_cur(1, 6);
	lcd_send_string("00:00:00");
	lcd_put_cur(1, 15);
	lcd_send_string("y");
	lcd_put_cur(1, 14);
	lcd_send_data(0x7F);
	lcd_put_cur(1, 6);
}

void alarm(int hour, int minute, int second) {
	if (check_time(hour, minute, second)) {
		HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
	}
}


/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

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
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	int i = 0;
	char s[3];
	int hour = 0;
	int minute = 0;
	int second = 0;
	int alarm_amount = 0;
//	itoa(i, s, 10);
	HAL_GPIO_Init(BUZZER_GPIO_Port, BUZZER_Pin);
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
//  HAL_GPIO_WritePin(BUZZER_2_GPIO_Port, BUZZER_2_Pin, 1);
//  HAL_GPIO_Init(BUTTON_GPIO_Port, BUTTON_Pin);
//  HAL_GPIO_Init(LED_GPIO_Port, LED_Pin);
	lcd_init();
//	lcd_put_cur(0, 0);
//	lcd_send_string(s);
//  if (HAL_GPIO_ReadPin(BUZZER_GPIO_Port, BUZZER_Pin) == 0) {
//	  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
//  }
//  HAL_TIM_Base_Start_IT(&htim2);

	//Set_Time(00, 48, 12, 5, 18, 4, 24);
//  	if (check_time(hour, minute, second) == true) {
//		HAL_TIM_Base_Start_IT(&htim2);
//	}
//	if (time.seconds == second + 10) {
//		HAL_TIM_Base_Stop_IT(&htim2);
//	}
//	itoa(hour, s, 10);
//	lcd_send_string(s);
//	lcd_put_cur(1, 4);
//	itoa(minute, s, 10);
//	lcd_send_string(s);
//	lcd_put_cur(1, 10);
//	itoa(second, s, 10);
//	lcd_send_string(s);
//	lcd_put_cur(1, 13);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		status_button_1 = HAL_GPIO_ReadPin(BUTTON_1_GPIO_Port, BUTTON_1_Pin);
		status_button_2 = HAL_GPIO_ReadPin(BUTTON_2_GPIO_Port, BUTTON_2_Pin);
		status_button_3 = HAL_GPIO_ReadPin(BUTTON_3_GPIO_Port, BUTTON_3_Pin);
		status_button_stop = HAL_GPIO_ReadPin(BUTTON_STOP_ALARM_GPIO_Port,
		BUTTON_STOP_ALARM_Pin);
//		status_button_number = HAL_GPIO_ReadPin(BUTTON_NUMBER_GPIO_Port,
//		BUTTON_NUMBER_Pin);
		if (status_button_1 == 0) {
//			HAL_Delay(1000);
			lcd_clear();
			first_menu();
			lcd_put_cur(1, 6);
			lcd_send_cmd(0xF);
			while (1) {
				status_button_return = HAL_GPIO_ReadPin(BUTTON_RETURN_GPIO_Port,
				BUTTON_RETURN_Pin);
				status_button_right = HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port,
				BUTTON_RIGHT_Pin);
				status_button_reset = HAL_GPIO_ReadPin(BUTTON_BACK_GPIO_Port,
				BUTTON_BACK_Pin);
				status_button_number = HAL_GPIO_ReadPin(BUTTON_NUMBER_GPIO_Port,
				BUTTON_NUMBER_Pin);

				if (cur == 15) {
					lcd_clear();
					lcd_put_cur(0, 0);
					lcd_send_string("FINISHED!");
					lcd_put_cur(1, 0);
					hour = alarm_num[1] * 10 + alarm_num[2];
					minute = alarm_num[3] * 10 + alarm_num[4];
					second = alarm_num[5] * 10 + alarm_num[6];
					if (hour >= 24) {
						hour -= 24;
					} else
						hour -= 0;
					HAL_Delay(2000);
					lcd_clear();
					lcd_put_cur(1, 11);
					lcd_send_string("!");
					cur = 6;
					break;
				}
				if (status_button_right == 0) {
					//					if (cur == 6) {
					//						alarm_num[i] = num_i;
					//					}
					num_i = 0;
					if (cur == 7)
						cur = 9;
					else if (cur == 10)
						cur = 12;
					else
						cur++;
					lcd_put_cur(1, cur);
					lcd_send_cmd(0xF);
					//					if (cur == 8 || cur == 11 || cur == 14 || cur == 15) {
					//						i += 0;
					//					} else {
					if (i == 9) {
						i = 0;
					} else i++;
					//					}
					HAL_Delay(500);
				}
				if (status_button_reset == 0 && cur == 14) {
					cur = 6;
					lcd_put_cur(1, cur);
					lcd_send_cmd(0xF);
					HAL_Delay(1000);
				}
				if (status_button_return == 0) {
					lcd_clear();
					break;
				}

				if (status_button_number == 0) {
					HAL_Delay(500);
					num_i++;
					lcd_put_cur(1, cur);
					switch (cur) {
					case 6:
						//
						switch (num_i) {
						case 1:
						case 2:
							char temp[3];
							itoa(num_i, temp, 10);
							lcd_send_string(temp);
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							alarm_num[i + 1] = num_i;
							break;
						default:
							num_i = 0;
							lcd_send_string("0");
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							alarm_num[i + 1] = num_i;
							break;
						}
						//
						break;
					case 7:
					case 10:
					case 13:
						//
						switch (num_i) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:
							char temp[3];
							itoa(num_i, temp, 10);
							lcd_send_string(temp);
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							alarm_num[i + 1] = num_i;
							break;
						default:
							num_i = 0;
							lcd_send_string("0");
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							alarm_num[i + 1] = num_i;
							break;
						}
						//
						break;
					case 9:
					case 12:
						switch (num_i) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
							char temp[3];
							itoa(num_i, temp, 10);
							lcd_send_string(temp);
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							alarm_num[i + 1] = num_i;
							break;
						default:
							num_i = 0;
							lcd_send_string("0");
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							alarm_num[i + 1] = num_i;
							break;
						}
						break;
					default:
						lcd_put_cur(1, cur);
						alarm_num[i + 1] = num_i;
						break;
					}
				}
			}
		}

//		if (check_time(hour, minute, second)) {
//			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
//			if (status_button_stop == 0) {
//				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
//			}
//		}
		//
		/*
		 * int num = 0;
		 * if (HAL_GPIO_ReadPin(button) == )
		 * */
//			break;
		if (status_button_2 == 0) {
			lcd_clear();
			second_menu();
			lcd_put_cur(1, 6);
			lcd_send_cmd(0xF);
			while (1) {
				status_button_return = HAL_GPIO_ReadPin(BUTTON_RETURN_GPIO_Port,
				BUTTON_RETURN_Pin);
				status_button_right = HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port,
				BUTTON_RIGHT_Pin);
				status_button_reset = HAL_GPIO_ReadPin(BUTTON_BACK_GPIO_Port,
				BUTTON_BACK_Pin);
				status_button_number = HAL_GPIO_ReadPin(BUTTON_NUMBER_GPIO_Port,
				BUTTON_NUMBER_Pin);

				if (cur == 15) {
					lcd_clear();
					lcd_put_cur(0, 0);
					lcd_send_string("FINISHED!");
					lcd_put_cur(1, 0);
					//					for (int j = 0; j < 8; j++) {
					//						itoa(alarm_num[j], s, 10);
					//						lcd_send_string(s);
					//						lcd_put_cur(1, j);
					//					}

					HAL_Delay(2000);
					lcd_clear();
					cur = 6;
					break;
				}
				if (status_button_reset == 0 && cur == 14) {
					cur = 6;
					lcd_put_cur(1, cur);
					lcd_send_cmd(0xF);
					HAL_Delay(1000);
				}
				if (status_button_return == 0) {
					lcd_clear();
					break;
				}
				if (status_button_right == 0) {
					//					if (cur == 6) {
					//						alarm_num[i] = num_i;
					//					}
					num_i = 0;
					if (cur == 7)
						cur = 9;
					else if (cur == 10)
						cur = 12;
					else
						cur++;
					lcd_put_cur(1, cur);
					lcd_send_cmd(0xF);
					//					if (cur == 8 || cur == 11 || cur == 14 || cur == 15) {
					//						i += 0;
					//					} else {
					//					if (i == 9) {
					//						i = 0;
					//					} else
					//						i++;
					//					}
					HAL_Delay(250);
				}

				if (status_button_number == 0) {
					HAL_Delay(500);
					num_i++;
					lcd_put_cur(1, cur);
					switch (cur) {
					case 6:
						//
						switch (num_i) {
						case 1:
						case 2:
							char temp[3];
							itoa(num_i, temp, 10);
							lcd_send_string(temp);
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							//							alarm_num[i + 1] = num_i;
							break;
						default:
							num_i = 0;
							lcd_send_string("0");
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							//							alarm_num[i + 1] = num_i;
							break;
						}
						//
						break;
					case 7:
					case 10:
					case 13:
						//
						switch (num_i) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:
							char temp[3];
							itoa(num_i, temp, 10);
							lcd_send_string(temp);
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							//							alarm_num[i + 1] = num_i;
							break;
						default:
							num_i = 0;
							lcd_send_string("0");
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							//							alarm_num[i + 1] = num_i;
							break;
						}
						//
						break;
					case 9:
					case 12:
						switch (num_i) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
							char temp[3];
							itoa(num_i, temp, 10);
							lcd_send_string(temp);
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							//							alarm_num[i + 1] = num_i;
							break;
						default:
							num_i = 0;
							lcd_send_string("0");
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xE);
							//							alarm_num[i + 1] = num_i;
							break;
						}
						break;
					default:
						lcd_put_cur(1, cur);
						//						alarm_num[i + 1] = num_i;
						break;
					}
				}
			}
			 uint8_t target_hour, target_minute;
			    uint8_t current_hour, current_minute;

			    // Nhập thời gian muốn đếm ngược
			    printf("Nhap gio muon dat den (0-23): ");
			    scanf("%hhu", &target_hour);
			    printf("Nhap phut muon dat den (0-59): ");
			    scanf("%hhu", &target_minute);

			    // Đọc thời gian hiện tại từ DS3231
			    DS3231_GetTime(&current_hour, &current_minute);

			    // Chuyển đổi thời gian đến định dạng phút tính từ nửa đêm
			    uint16_t current_total_minutes = current_hour * 60 + current_minute;
			    uint16_t target_total_minutes = target_hour * 60 + target_minute;

			    // Tính số phút cần đếm ngược
			    int16_t countdown_minutes = target_total_minutes - current_total_minutes;

			    if (countdown_minutes <= 0) {
			        printf("Thoi gian da nhap da troi qua hoac la trung voi thoi gian hien tai.\n");
			        return 0;
			    }

			    printf("Bat dau dem nguoc tu %02d:%02d den %02d:%02d (%d phut).\n", current_hour, current_minute, target_hour, target_minute, countdown_minutes);

			    while (countdown_minutes > 0) {
			        HAL_Delay(60000);  // Delay 1 phút (60,000 ms)

			        // Cập nhật lại thời gian hiện tại
			        DS3231_GetTime(&current_hour, &current_minute);
			        current_total_minutes = current_hour * 60 + current_minute;

			        // Tính lại số phút cần đếm ngược
			        countdown_minutes = target_total_minutes - current_total_minutes;

			        printf("Con lai %d phut.\n", countdown_minutes);
			    }

			    printf("Dem nguoc hoan tat den thoi diem da nhap.\n");

			    while (1) {
			        // Vòng lặp vô hạn sau khi đếm ngược hoàn tất
			    }
			}
		}
		if (status_button_3 == 0) {
		           	lcd_clear();
					first_menu();
					lcd_put_cur(1, 6);
					lcd_send_cmd(0xF);
					while (1) {
						status_button_return = HAL_GPIO_ReadPin(BUTTON_RETURN_GPIO_Port,
										BUTTON_RETURN_Pin);
										status_button_right = HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port,
										BUTTON_RIGHT_Pin);
										status_button_reset = HAL_GPIO_ReadPin(BUTTON_BACK_GPIO_Port,
										BUTTON_BACK_Pin);
										status_button_number = HAL_GPIO_ReadPin(BUTTON_NUMBER_GPIO_Port,
										BUTTON_NUMBER_Pin);
										status_button_confirm = HAL_GPIO_ReadPin(BUTTON_CONFIRM_GPIO_Port,
													BUTTON_CONFIRM_Pin);

						if (status_button_confirm  == 0) {
							lcd_clear();
							lcd_put_cur(0, 0);
							lcd_send_string("FINISHED!");
							lcd_put_cur(1, 0);
							hour = alarm_num[1] * 10 + alarm_num[2];
							minute = alarm_num[3] * 10 + alarm_num[4];
							second = alarm_num[5] * 10 + alarm_num[6];
							if (hour >= 24) {
								hour -= 24;
							} else
								hour -= 0;
							HAL_Delay(2000);
							lcd_clear();
							lcd_put_cur(1, 11);
							lcd_send_string("!");
							cur = 6;
							break;
						}
						if (status_button_right == 0) {
							//					if (cur == 6) {
							//						alarm_num[i] = num_i;
							//					}
							num_i = 0;
							if (cur == 7)
								cur = 9;
							else if (cur == 10)
								cur = 12;
							else
								cur++;
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xF);
							//					if (cur == 8 || cur == 11 || cur == 14 || cur == 15) {
							//						i += 0;
							//					} else {
							if (i == 9) {
								i = 0;
							} else i++;
							//					}
							HAL_Delay(500);
						}
						if (status_button_reset == 0 && cur == 15) {
							cur = 6;
							lcd_put_cur(1, cur);
							lcd_send_cmd(0xF);
							HAL_Delay(1000);
						}
						if (status_button_return == 0) {
							lcd_clear();
							break;
						}

						if (status_button_number == 0) {
							HAL_Delay(500);
							num_i++;
							lcd_put_cur(1, cur);
							switch (cur) {
							case 6:
								//
								switch (num_i) {
								case 1:
								case 2:
									char temp[3];
									itoa(num_i, temp, 10);
									lcd_send_string(temp);
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_num[i + 1] = num_i;
									break;
								default:
									num_i = 0;
									lcd_send_string("0");
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_num[i + 1] = num_i;
									break;
								}
								//
								break;
							case 7:
							case 10:
							case 13:
								//
								switch (num_i) {
								case 1:
								case 2:
								case 3:
								case 4:
								case 5:
								case 6:
								case 7:
								case 8:
								case 9:
									char temp[3];
									itoa(num_i, temp, 10);
									lcd_send_string(temp);
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_num[i + 1] = num_i;
									break;
								default:
									num_i = 0;
									lcd_send_string("0");
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_num[i + 1] = num_i;
									break;
								}
								//
								break;
							case 9:
							case 12:
								switch (num_i) {
								case 1:
								case 2:
								case 3:
								case 4:
								case 5:
									char temp[3];
									itoa(num_i, temp, 10);
									lcd_send_string(temp);
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_num[i + 1] = num_i;
									break;
								default:
									num_i = 0;
									lcd_send_string("0");
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_num[i + 1] = num_i;
									break;
								}
								break;
								case 14:
									switch (num_i)
									{
									case 1:
																	case 2:
																	case 3:
																	case 4:
																	case 5:
																	case 6:
																	case 7:
																	case 8:
																	case 9:
									char temp[3];
									itoa(num_i, temp, 10);
									lcd_send_string(temp);
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_amount = num_i;
									break;
									default:
									num_i = 0;
									lcd_send_string("0");
									lcd_put_cur(1, cur);
									lcd_send_cmd(0xE);
									alarm_amount = num_i;
									break;

									}

							default:
								lcd_put_cur(1, cur);
								alarm_num[i + 1] = num_i;
								break;
							}
						}
					}
				}

		//		if (check_time(hour, minute, second)) {
		//			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
		//			if (status_button_stop == 0) {
		//				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
		//			}
		//		}
				//
				/*
				 * int num = 0;
				 * if (HAL_GPIO_ReadPin(button) == )
				 * */
		//			break;
		}
		update_Time();
		button_menu();
		if (time.hour == hour && time.minutes == minute
				&& time.seconds == second) {
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
			update_Time();
			button_menu();
		} else
			continue;
		while  (alarm_amount > -1 )
		{	if (time.hour == hour && time.minutes == minute
					&& time.seconds == second && alarm_amount ) {
				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
				update_Time();
				button_menu();
				alarm_amount --;
			} else
				continue;
		}
//		alarm();

//		if (status_button_2 == 0) {
//			second_menu();
//		}
//		if (status_button_3 == 0) {
//			second_menu();
//		}
	}
//	while (1) {
//		if (status_button_right == 0) {
//			lcd_send_cmd(0x14);
//		} else if (status_button_left == 0) {
//			lcd_send_cmd(0x10);
//		}
//		lcd_send_cmd(0xF);
//	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : BUTTON_3_Pin BUTTON_2_Pin BUTTON_1_Pin BUTTON_RIGHT_Pin
	 BUTTON_BACK_Pin */
	GPIO_InitStruct.Pin = BUTTON_3_Pin | BUTTON_2_Pin | BUTTON_1_Pin
			| BUTTON_RIGHT_Pin | BUTTON_BACK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : BUTTON_RETURN_Pin BUTTON_NUMBER_Pin BUTTON_STOP_ALARM_Pin */
	GPIO_InitStruct.Pin = BUTTON_RETURN_Pin | BUTTON_NUMBER_Pin
			| BUTTON_STOP_ALARM_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : BUZZER_Pin */
	GPIO_InitStruct.Pin = BUZZER_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

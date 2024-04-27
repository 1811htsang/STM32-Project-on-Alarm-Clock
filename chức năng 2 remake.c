void countime(int hour, int minutes, int second){
	int time = hour*3600 + minutes*60 + second;
	lcd_put_cur(0,0);
	lcd_send_string("count down time");
	while (time >0 ){
		if (time >= 3600){
			hour = time/3600;
			minutes = (time % 3600) / 60;
			second = time % 60;
			sprintf(buffer, "%02d:%02d:%02d", hour, minutes, second);
			lcd_put_cur(1, 0);
			lcd_send_string(buffer);
		}
		else if (time >= 60){
			hour = 00;
			minutes = time / 60;
			second = time % 60;
			sprintf(buffer, "%02d:%02d:%02d", hour, minutes, second);
			lcd_put_cur(1, 0);
			lcd_send_string(buffer);
		}
		else {
			hour = 00;
			minutes = 00;
			second = time / 60;
			sprintf(buffer, "%02d:%02d:%02d", hour, minutes, second);
			lcd_put_cur(1, 0);
			lcd_send_string(buffer);
		}
		time--;
		HAL_delay(1000);
	}
	if (hour == 0 && minutes == 0 && second == 0){
		Hal_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
	}
}

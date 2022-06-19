/*
1. AVR - A에서는 실내 온도 측정은 LM35을 이용하여 ADC4에 서 측정한다.

2. 보일러 설정 온도는 키보드에서 처리한다.

- SW4 누르면 설정 온도 증가

- SW5 누르면 설정 온도 감소

- SW6 누르면 25도로 고정된 설정

- SW7 누르면 0도로 설정)

>> 실내  :  Remote controllerAVR - A 을 위해 모든 PORT 입출력 방향을 DDR에서 설정하고, 이것을 INIT_DDR() 라는 함수로 구현
*/
​

INIT_DDR() {

	DDRB = 0xff;

	DDRC = 0xf0;

	DDRG = 0xff;

}

​

​
/*
>> 실내 : 키패드를 읽는 함수 int key_pad()

- SW4 누르면 보일러 설정 온도가 증가(설정 온도 변수 : temp(전역 변수))

- SW5 누르면 설정 온도 감소

- SW6 누르면 설정 온도를 25도로 설정

- SW7 누르면 0도 로 설정
*/
​

volatile unsigned int temp;

int key_pad() {

	PORTC = 0x02;

	​

		if (~PINC & 0x0f == 0x01) {

			temp++;

		}

		else if (~PINC & 0x0f == 0x02) {

			temp = 25;

		}

		else if (~PINC & 0x0f == 0x04) {

			temp = 0;

		}

}

​

​
/*
>> 실내 : unsigned char FND_value1, FND_value2는 전역 변수로 4자리 FND에 표시될 2자리 수.

이것을 표시할 함수 display_FND()을 구현(단, 4자리 표시에서 2자리만 사용)
*/
​

volatile unsigned char FND_value1, FND_value2;

unsigned char FND_value1[10] = { 0x3f, 0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f };

unsigned char FND_value2[10] = { 0x3f, 0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f };

​

void display_FND()() {

	int count = 0;

	count = [(count % 100) / 10];

	PORTC = ~FND_value1[count];

	count = [(count % 10) / 1];

	PORTC = ~FND_value2[count];

}

​

​
/*
//실내 : 시리얼 초기화 루틴을 만들어라 보오레이트는 9600, 함수 void serial_init(unsigned long baud) 을 만들기
*/
​

void serial_init(unsigned long baud) {

	unsigned short ubrr;

	ubrr = (unsigned short)(F_CPU / (16 * baud) - 1);

	UBRR0H = (unsigned char)(ubrr >> 8); //상위 비트

	UBRR0L = (unsigned char)(ubrr & 0xff); //하위 비트

	UCSR0A = 0x00;

	UCSR0B = 0x18; //인터럽트 X, 수신부 가능, 송신부 가능

	UCSR0C = 0x06; //비동기 통신, 패리티비트 없음, 정지비트 : 1, 문자 : 8

}

​

​
/*
>> 실내 : 실내온도 즉 내부 온도는 ADC4에서 받고, 이것을 읽어 들이는 get_adc() 함수를 만들기
*/
​

unsigned int get_adc(unsigned char CH) {

	unsigned int adc_value;

	ADCSRA = 0xc7; //ADC가능, ADC변환시작, Free Running모드-> off

	ADMUX = 0x10;

	ADMUX = CH | 0x40 //ADC채널 CH번 사용 (ADC4)

		while ((ADCSRA & 0x10) == 0x00); //변환종료를 기다림 

	adc_value = ADC;

	ADCSRA = ADCSRA | 0x10; //리셋 

	return(adc_value);

}

​

​
/*
>> 실내 : AVR128 - B로 요청(ReadyToSend)을 받으면 설정 온도와 실내 온도를 AVR128 - B 에 전송하는 함수 만들기(설정 온도와 실내 온도는 각 각 2개의 아스키 코드로 되어 있음) 이것을 구현하는 send_temp(, , , , )을 프로그램 만들기
*/
​

void send_temp() {

	while (1) {

		adc_value = get_adc(4);

		temperature = get_adc(4);

		​

			FND_display(adc_value); //보일러 온도값

		​

			if (adc_value > 100) adc_value = 99;

		tx_data((adc_value % 100) / 10 + ’0’);

		tx_data((adc_value % 10) / 1 + ’0’);

		tx_data('\n');

		tx_data((temperature % 100) / 10 + ’0’);

		tx_data((temperature % 10) / 1 + ’0’);

		tx_data('\n');

	}

}

​
/*
>> 실외 : AVR128 - B에서 원격 보일러에서 수신하여 제어하는 프로그램을 작성한다.AVR128 - A로 요청(ReadyToSend)을 한 후, 보일러 설정 온도와 실내 온도를 받고 보일러를 제어
*/
​

#include <avr/io.h>

#include <util/delay.h>

#define F_CPU 7372800UL

unsigned char FND[10] = { 0x3f, 0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f };

​

void serial_init(unsigned long baud) {

	unsigned short ubrr;

	ubrr = (unsigned short)(CPU_CLOCK_HZ / (16 * baud) - 1);

	UBRR0H = (unsigned char)(ubrr >> 8);

	UBRR0L = (unsigned char)(ubrr & 0xff);

	UCSR0C = 0x06; // 비동기 , Stop : 1bit , 8bit , 패리티X

	UCSR0B = 0x18; // RX TX 허용

}

​

void tx_data(unsigned char data) {

	while ((UCSR0A & 0x20) == 0x00);

	UDR0 = data;

}

​

unsigned char rx_data(void) {

	while ((UCSR0A & 0x80) == 0x00);

	return UDR0;

}

​

void display_FND()() {

	int count = 0;

	count = [(count % 100) / 10];

	PORTC = ~FND_value1[count];

	count = [(count % 10) / 1];

	PORTC = ~FND_value2[count];

}

​

unsigned int get_ADC(unsigned char ch) {

	unsigned int adc_value;

	​

		ADCSRA = 0x87;

	ADMUX = ch | 0xc0;

	ADCSRA |= 1 << ADSC;

	​

		while (ADCSRA & (1 << 0x10 == 0x00); //변환이 완료될때까지 대기

	adc_value = ADC;

	ADCSRA = ADCSRA | 0x10; //초기화

	return adc_value; //ADC값 반환

}

int main(void) { //송신

	DDRA = 0xff;

	DDRB = 0xff;

	​

		unsigned int adc_value, temperature;

	​

		serial_init(9600);

	​

		while (1) {

			adc_value = get_adc(0);

			adc_value = (adc_value >> 2);

			temperature = get_adc(7);

			temperature = (temperature >> 2);

			​

				FND_display(adc_value); //보일러 온도값

			​

				if (adc_value > 100) adc_value = 99;

			tx_data((adc_value % 100) / 10 + ’0’);

			tx_data((adc_value % 10) / 1 + ’0’);

			tx_data('\n');

			tx_data((temperature % 100) / 10 + ’0’);

			tx_data((temperature % 10) / 1 + ’0’);

			tx_data('\n');

		}

	​

		void main() { //수신

		unsigned char adc_value;

		​

			DDRA = 0xFF; //FND 10의자리

		DDRB = 0xFF; //FND 1의자리

		DDRC = 0xff; //보일러 ON/OFF

		​

			serial_init(9600);

		​

			while (1) {

				tx_data(ReadyToSend);

				​

					temp_10 = rx_data() - '0';

				temp_10 = temp10 * 10;

				​

					temp_1 = rx_data() - '0';

				​

					rx_data();

				temp = temp_10 + temp_1;

				​

					FND_setting(temp);

				​

					if (temp < 20)

						PORTC = ~0x01;

					else

						PORTC = ~0xff;

				​

			}

	}
# ProyekMikro2

Proyek ini dibuat untuk memenuhi kelulusan dari mata kuliah Embedded System, dengan membuat sebuah sistem kecil yang memanfaatkan penggunaan ESP8266 NodeMCU. Pada sistem ini digunakan NodeJS sebagai web service, MQTT Broker yang disediakan oleh Eclipse (https://mosquitto.org/download/), MySQL yang disediakan oleh XAMPP secara lokal, dan Arduino IDE untuk melakukan pemrograman pada NodeMCU. Semua terhubung menggunakan WIFI yang tersambung dengan jaringan lokal. Pada proyek ini terdapat input sebagai berikut. Input digital yang digunakan adalah sensor suhu dan kelembaban DHT11, dan Input analog yang digunakan adalah potensiometer dan pushbutton. Sedangkan output dihasilkan oleh 2(dua) buah LED berwarna biru dan merah, dan buzzer untuk menghasilkan suara.

No.	KomponenPin	Keterangan
1.	Buzzer&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;D5	
2.	DHT11&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;D4	Diberi resistor pada data.
3.	LED Merah&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;D6	Diberi resistor.
3. 	LED Biru&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;D7	
4.	LCD 16x2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;D1, D2, VU	Menggunakan pin SCL dan SDA, dan VU untuk power in.
5.	Potensiometer	A0	
6.	Pushbutton&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;D8	

#include <QMC5883LCompass.h>
#include <Stepper.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define satuButton 2
#define duaButton 3

const int stepsPerRevolution = 2048;
static const int RXPin = 13, TXPin = 12;
static const uint32_t GPSBaud = 9600;

QMC5883LCompass compass;
LiquidCrystal_I2C lcd(0x27,16,2);
Stepper stepperDua = Stepper(stepsPerRevolution, 8, 10, 9, 11);
Stepper stepperSatu = Stepper(stepsPerRevolution, 4, 6, 5, 7);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

bool firstButtonPressedTemp;
bool secondButtonPressedTemp;
bool bothButtonPressedTemp;
bool tekanSatu, tekanDua;
bool keduaTombol;
bool changed = false;
bool done = false;
bool calibrated = false;
bool sudah = false;
bool sudahDua = false;
double konfer = 5.688888888888889;
float altitudeBulan;
double tahunGPS, bulanGPS, hariGPS, jamGPS, menitGPS, detikGPS, bujurGPS, lintangGPS;

//pengaturan HILAL
float azimutBulan = 113.81;
float altitudeBulanLama = 82.46;
int range = 5; //berapakali dia tracker
// int delayPer = 1000;//1 detik = 1000, 10000 = 10 detik
//double pergerakanBulan = 0.21506; //1 menit
// double pergerakanBulan = 0.0359; //10 detik
// double pergerakanBulan = 0.00359; //1 detik
double pergerakanBulan = 0.0008975; //1/4 detik
//double pergerakanBulan = 0.0539; //15 detik

double tahunGPS, bulanGPS, hariGPS, jamGPS, menitGPS, detikGPS, bujurGPS, lintangGPS;

int modulCompass(){
  int kalibrasi[10];
  int total = 0;
  int kalibrasiAkhir = 0;
  for (int i = 0; i < 10; i++) {
      compass.read();
      int azimutz = compass.getAzimuth();
      kalibrasi[i] = azimutz;
      total += kalibrasi[i];
  }
  kalibrasiAkhir = total / 10;
  return kalibrasiAkhir;
}

void serialScreen(int kalibrasiAkhirA, int gpsTahun, int gpsBulan, int gpsHari, int gpsJam, int gpsMenit, int gpsDetik, double gpsBujur, double gpsLintang){
  int Jam = gpsJam + 7;
  if (Jam >= 24){
    Jam = Jam - 24;
    gpsHari = gpsHari + 1;
  }
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Azimut Alat:");
  lcd.setCursor(13,0);
  lcd.print(kalibrasiAkhirA);
  delay(2000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Lintang Lokasi: ");
  lcd.setCursor(0,1);
  lcd.print(gpsLintang, 9);
  delay(2000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Bujur Lokasi: ");
  lcd.setCursor(0,1);
  lcd.print(gpsBujur, 9);
  delay(2000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Tanggal : ");
  lcd.setCursor(0,1);
  lcd.print(gpsHari);
  lcd.setCursor(2,1);
  lcd.print(" : ");
  lcd.setCursor(5,1);
  lcd.print(gpsBulan);
  lcd.setCursor(7,1);
  lcd.print(" : ");
  lcd.setCursor(10,1);
  lcd.print(gpsTahun);
  delay(2000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Time : ");
  lcd.setCursor(0,1);
  lcd.print(Jam);
  lcd.setCursor(2,1);
  lcd.print(" : ");
  lcd.setCursor(5,1);
  lcd.print(gpsMenit);
  lcd.setCursor(7,1);
  lcd.print(" : ");
  lcd.setCursor(10,1);
  lcd.print(gpsDetik);
  delay(2000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Saat Alat       ");
  lcd.setCursor(0,1);
  lcd.print("Sedang Berjalan-");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Jangan Dimatikan");
  lcd.setCursor(0,1);
  lcd.print(" / Dicabut Paksa");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Silahkan Pilih  ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Putih: Kiblat   ");
  lcd.setCursor(0,1);
  lcd.print("Hitam: Hilal     ");
}

//KIBLAT
void stepperMotorPertama(int kalibrasiAkhirB, double sudutArahKiblatB){
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Sudut Arah      ");
  lcd.setCursor(0,1);
  lcd.print("Kiblat: ");
  lcd.setCursor(9,1);
  lcd.print(sudutArahKiblatB,3);
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alat Bergerak");
  lcd.setCursor(0,1);
  lcd.print("Ke Utara Sejati ");
  delay(2000);
  stepperSatu.step(konfer*-kalibrasiAkhirB);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menghadap Titik ");
  lcd.setCursor(0,1);
  lcd.print("Utara Sejati");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alat Bergerak");
  lcd.setCursor(0,1);
  lcd.print("Ke Arah Kiblat");
  stepperSatu.step(konfer*(sudutArahKiblatB+5));
  stepperDua.step(konfer*20);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Telah Menghadap");
  lcd.setCursor(0,1);
  lcd.print("Ke Arah Kiblat");
  while (tekanSatu == false){
    bool satuButtonState = digitalRead(satuButton);
      if (keduaTombol == true && satuButtonState == LOW && firstButtonPressedTemp == false) {
          firstButtonPressedTemp = true;
          tekanSatu = false;
          } 
      if (keduaTombol == true && satuButtonState == HIGH && firstButtonPressedTemp == true) {
          firstButtonPressedTemp = false;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Alat Bergerak");
          lcd.setCursor(0,1);
          lcd.print("Kembali Ke Asal");
          stepperDua.step(konfer*-20);
          stepperSatu.step(konfer*(-sudutArahKiblatB-5));
          stepperSatu.step(konfer*kalibrasiAkhirB);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Putih: Kiblat   ");
          lcd.setCursor(0,1);
          lcd.print("Hitam: Hilal     ");
          tekanSatu = true;
          }
      delay(10);
  }
}

// HILAL
void stepperMotorDua(int kalibrasiAkhirC){
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alat Bergerak");
  lcd.setCursor(0,1);
  lcd.print("Ke Utara Sejati ");
  delay(1000);
  stepperSatu.step(konfer*-kalibrasiAkhirC);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menghadap Titik ");
  lcd.setCursor(0,1);
  lcd.print("Utara Sejati");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alat Bergerak");
  lcd.setCursor(0,1);
  lcd.print("Ke Azimut Bulan ");
  stepperSatu.step(konfer*azimutBulan);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ke Altitude");
  lcd.setCursor(0,1);
  lcd.print("Bulan");

  //statement alt
  if(altitudeBulanLama >= 0){
    altitudeBulan = -altitudeBulanLama;
    stepperDua.step(konfer*altitudeBulan);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Posisi Bulan");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Az =");
    lcd.setCursor(6,0);
    lcd.print(azimutBulan,5);
    lcd.setCursor(0,1);
    lcd.print("Alt =");
    lcd.setCursor(7,1);
    lcd.print(-(altitudeBulan), 5);
    delay(1000);
      while (tekanDua == false){
        bool duaButtonState = digitalRead(duaButton);
        altitudeBulan = altitudeBulan - pergerakanBulan;
        stepperDua.step(konfer*-pergerakanBulan);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Az =");
        lcd.setCursor(6,0);
        lcd.print(azimutBulan,5);
        lcd.setCursor(0,1);
        lcd.print("Alt =");
        lcd.setCursor(7,1);
        lcd.print(-(altitudeBulan), 5);
          if (keduaTombol == true && duaButtonState == LOW && secondButtonPressedTemp == false) {
            secondButtonPressedTemp = true;
            } 
          if (keduaTombol == true && duaButtonState == HIGH && secondButtonPressedTemp == true) {
            secondButtonPressedTemp = false;
            delay(3000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Alat Bergerak");
            lcd.setCursor(0,1);
            lcd.print("Ke Posisi Awal");
            stepperDua.step(konfer*-altitudeBulan);
            stepperSatu.step(konfer*-azimutBulan);
            stepperSatu.step(konfer*kalibrasiAkhirC);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Putih: Kiblat   ");
            lcd.setCursor(0,1);
            lcd.print("Hitam: Hilal     ");
            tekanDua = true;
            }
        delay(250);
      }
  } else {

    altitudeBulan = -(altitudeBulanLama);
    stepperDua.step(konfer*altitudeBulan);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Posisi Bulan");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Az =");
    lcd.setCursor(6,0);
    lcd.print(azimutBulan);
    lcd.setCursor(0,1);
    lcd.print("Alt =");
    lcd.setCursor(7,1);
    lcd.print(-altitudeBulan, 5);
    delay(3000);
      while (tekanDua == false){
        bool duaButtonState = digitalRead(duaButton);
        altitudeBulan = altitudeBulan + pergerakanBulan;
        stepperDua.step(konfer*pergerakanBulan);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Az =");
        lcd.setCursor(6,0);
        lcd.print(azimutBulan);
        lcd.setCursor(0,1);
        lcd.print("Alt =");
        lcd.setCursor(7,1);
        lcd.print(-altitudeBulan, 5);
          if (keduaTombol == true && duaButtonState == LOW && secondButtonPressedTemp == false) {
            secondButtonPressedTemp = true;
            } 
          if (keduaTombol == true && duaButtonState == HIGH && secondButtonPressedTemp == true) {
              secondButtonPressedTemp = false;
              delay(3000);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Alat Bergerak");
              lcd.setCursor(0,1);
              lcd.print("Ke Posisi Awal");
              stepperDua.step(konfer*-altitudeBulan);
              stepperSatu.step(konfer*-azimutBulan);
              stepperSatu.step(konfer*kalibrasiAkhirC);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Putih: Kiblat   ");
              lcd.setCursor(0,1);
              lcd.print("Hitam: Hilal     ");
              tekanDua = true;
            }
          delay(250);
        }
    }
}

void setup() {
  lcd.backlight();
  Serial.begin(9600);
  ss.begin(GPSBaud);
  lcd.init();
  lcd.init();
  compass.init();
  stepperSatu.setSpeed(5);
  stepperDua.setSpeed(5);
  pinMode(satuButton, INPUT_PULLUP);
  pinMode(duaButton, INPUT_PULLUP);
  keduaTombol = false;
}
 
void loop() {
while (ss.available() > 0){
  gps.encode(ss.read());
  if (gps.location.isUpdated() == 1){
    if (gps.date.year() == 2023){
      if (sudahDua == false){
        tahunGPS = gps.date.year();
        bulanGPS = gps.date.month();
        hariGPS = gps.date.day();
        jamGPS = gps.time.hour();
        menitGPS = gps.time.minute();
        detikGPS = gps.time.second();
        bujurGPS = gps.location.lng();
        lintangGPS = gps.location.lat();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("READY");
        sudahDua = true;
      }
    }
  }
}
if(sudahDua == true){
  bool satuButtonState = digitalRead(satuButton);
  bool duaButtonState = digitalRead(duaButton);
  if(satuButtonState == LOW && duaButtonState == LOW && keduaTombol == false && bothButtonPressedTemp == false){
      bothButtonPressedTemp = true;
      }
  if (satuButtonState == HIGH && duaButtonState == HIGH && bothButtonPressedTemp == true) {
      keduaTombol = true;
      bothButtonPressedTemp = false;
      if (sudah == false){
          modulCompass();
          perhitunganArahKiblat(lintangGPS, bujurGPS);
          stepperSatu.step(1);
          stepperDua.step(-1);
          stepperSatu.step(1);
          stepperDua.step(-1);
          serialScreen(modulCompass(), tahunGPS, bulanGPS, hariGPS, jamGPS, menitGPS, detikGPS, bujurGPS, lintangGPS);
          sudah = true;
          }
      }
  //button untuk arah kiblat
      if (keduaTombol == true && satuButtonState == LOW && firstButtonPressedTemp == false) {
          firstButtonPressedTemp = true;
          tekanSatu = false;
          } 
      if (keduaTombol == true && satuButtonState == HIGH && firstButtonPressedTemp == true) {
          firstButtonPressedTemp = false;
          stepperMotorPertama(modulCompass(), perhitunganArahKiblat(gps.location.lat(), gps.location.lng()));
          }
  //button untuk arah hilal
      if (keduaTombol == true && duaButtonState == LOW && secondButtonPressedTemp == false) {
          secondButtonPressedTemp = true;
          tekanDua = false;
      } 
      if (keduaTombol == true && duaButtonState == HIGH && secondButtonPressedTemp == true) {
          secondButtonPressedTemp = false;
          stepperMotorDua(modulCompass());
      }
  delay(10);
}
}
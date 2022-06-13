/*
NAPOMENA:
NE PRITISKATI DUGMAD PREBRZO!!!!
*/

#include "mbed.h"
#include "stm32f413h_discovery_lcd.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"


#include <vector>
#include <string>


#define MQTT_CLIENT_NAME "US_Projekat_Kasa_MBED"
#define TEMAKUPOVINA "projekatkasa/kasa/kod"
#define TEMAUNOS "projekatkasa/kasa/unos"
#define TEMABRISANJE "projekatkasa/kasa/brisanje"

enum Stanja {
    POCETNO = 0,
    KUPOVINA,
    PLACANJE,
    UNOS,
    BRISANJE
};


TS_StateTypeDef TS_State = { 0 };
InterruptIn nazadNaPocetno(BUTTON1);
InterruptIn taster_p5(p5);
InterruptIn taster_p6(p6);
InterruptIn taster_p7(p7);
InterruptIn taster_p8(p8);
InterruptIn taster_p9(p9);
AnalogIn potenciometar(p20);
PwmOut ledice[5] = {p10, p11, p12, p13, p14};

float iznosRacuna=0;
float kolicinaArtikla = 1.;
char trenutnoStanje=POCETNO;
//ne moze iz interrupta refreshovati ekran
//vec ce taj interrupt ovaj int postaviti na +-1
//pa ce se u mainu pozvati refresh ekrana
int promijenjenaKolicina = 0;

typedef struct Artikal {
    std::string barkod, naziv;
    float cijena;
    
    Artikal(std::string _barkod = "", std::string _naziv = "", float _cijena = 0.) {
        barkod = _barkod;
        naziv = _naziv;
        cijena = _cijena;
    }
    
} Artikal;

Artikal testArtikal("AAAAAAAA", "test", 25);
std::vector<Artikal> sviArtikli;
Artikal skeniraniArtikal;


void kupovina_stanje();
void brisanje_stanje();
void unos_stanje();
void brisanje_stanje();
void placanje_stanje();
void gasi_ledice();

void pocetno_stanje(){
    taster_p5.fall(&kupovina_stanje);
    taster_p6.fall(&brisanje_stanje);
    taster_p7.fall(&unos_stanje);
    gasi_ledice();
    
    trenutnoStanje=POCETNO;
    iznosRacuna=0;

    BSP_LCD_Clear(LCD_COLOR_WHITE);

    BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"Odaberite opciju:", CENTER_MODE);
    
    
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
    BSP_LCD_FillRect(0, 40, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 55, (uint8_t *)"Kupovina", CENTER_MODE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawHLine(0,40,BSP_LCD_GetXSize());
    
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
    BSP_LCD_FillRect(0, 80, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 95, (uint8_t *)"Brisanje artikla", CENTER_MODE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawHLine(0,80,BSP_LCD_GetXSize());
    
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
    BSP_LCD_FillRect(0, 120, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 135, (uint8_t *)"Unos artikla", CENTER_MODE);
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawHLine(0,120,BSP_LCD_GetXSize());
    
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 180, (uint8_t *)"US Projekat", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 210, (uint8_t *)"Elvir - Vedad - Tarik", CENTER_MODE);
}

void povecaj_kolicinu() {
    //if (trenutnoStanje != KUPOVINA) return;
    promijenjenaKolicina = 1;
    kolicinaArtikla += 1.f;
}

void smanji_kolicinu() {
    //if (trenutnoStanje != KUPOVINA) return;
    promijenjenaKolicina = -1;
    kolicinaArtikla -= 1.f;
    if (kolicinaArtikla < 0.) {
        kolicinaArtikla = 0.f;    
    }
}

void decimalno_povecaj_kolicinu() {
    //if (trenutnoStanje != KUPOVINA) return;
    kolicinaArtikla += potenciometar;
    promijenjenaKolicina = 1;
}

void decimalno_smanji_kolicinu() {
    //if (trenutnoStanje != KUPOVINA) return;
    kolicinaArtikla -= potenciometar;
    promijenjenaKolicina = -1;
}

void kupovina_stanje(){
    if(trenutnoStanje == POCETNO){
        trenutnoStanje=KUPOVINA;
        
        taster_p5.fall(&povecaj_kolicinu);
        taster_p6.fall(&smanji_kolicinu);
        taster_p7.fall(&decimalno_povecaj_kolicinu);
        taster_p8.fall(&decimalno_smanji_kolicinu);
        taster_p9.fall(&placanje_stanje);
        
        iznosRacuna=0;
        kolicinaArtikla = 1;
        std::string temp="Skenirajte artikal!";
        BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
        BSP_LCD_DisplayStringAt(0, 5, (uint8_t *)"Kupovina", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 60,(uint8_t *)temp.c_str(), CENTER_MODE);
        printf("Dostupni: \n");
        for(char i = 0; i < sviArtikli.size(); ++i){
            printf("%s\n", sviArtikli.at(i).naziv.c_str());
        }
    }
    //u slucaju povecavanja/smanjivanja kolicine ovaj uslov ce biti ispunjen
    else if (trenutnoStanje == KUPOVINA && promijenjenaKolicina != 0) {
        BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_DisplayStringAt(0, 60, (uint8_t *) "                      ", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 90, (uint8_t *) "                      ", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 150, (uint8_t *) "                      ", LEFT_MODE);
        
        BSP_LCD_DisplayStringAt(0, 60, (uint8_t*)(std::to_string(skeniraniArtikal.cijena * kolicinaArtikla).substr(0,5) + " KM").c_str(), CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)("Kolicina: " + std::to_string(kolicinaArtikla).substr(0,5)).c_str(), CENTER_MODE);
        
        std::string temp = "Ukupno: " + std::to_string(iznosRacuna).substr(0, 5) + " KM";
        
        BSP_LCD_DisplayStringAt(0, 150, (uint8_t *)temp.c_str(), LEFT_MODE);
    }
}

void placanje_stanje() {
    if(trenutnoStanje==KUPOVINA){
        trenutnoStanje=PLACANJE;
        iznosRacuna += skeniraniArtikal.cijena * kolicinaArtikla;
        std::string temp = std::to_string(iznosRacuna).substr(0, 5) + " KM";
        BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
        BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Konacni iznos racuna:", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 130, (uint8_t *)temp.c_str(), CENTER_MODE);
        
        skeniraniArtikal = Artikal();
    }
}

void unos_stanje(){
    if(trenutnoStanje != UNOS){
        trenutnoStanje=UNOS;
        BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Unos artikala", CENTER_MODE);
    }
}

void brisanje_stanje(){
    if(trenutnoStanje!=BRISANJE){
        trenutnoStanje=BRISANJE;
        BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Brisanje artikala", CENTER_MODE);
    }
}

void mqtt_stigao_skenirani_artikal(MQTT::MessageData& md)
{
    if(trenutnoStanje==KUPOVINA){
        iznosRacuna += skeniraniArtikal.cijena * kolicinaArtikla;
        kolicinaArtikla = 1.f;
        
        BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
        BSP_LCD_DisplayStringAt(0, 5, (uint8_t *)"Kupovina", CENTER_MODE);
        MQTT::Message& message = md.message;

        std::string payload = (char*)message.payload;
        payload.resize(message.payloadlen);
        
        bool nadjen = false;
        
        std::string ispis;
        
        for (char i = 0; i < sviArtikli.size(); ++i) {
            Artikal temp = sviArtikli.at(i);
            if (payload == temp.barkod) {
                nadjen = true;
                skeniraniArtikal = temp;
                
                BSP_LCD_DisplayStringAt(0, 30, (uint8_t*)temp.naziv.c_str(), CENTER_MODE);
                BSP_LCD_DisplayStringAt(0, 60, (uint8_t*)(std::to_string(temp.cijena).substr(0, 5) + " KM").c_str(), CENTER_MODE);
                BSP_LCD_DisplayStringAt(0, 90, (uint8_t*)("Kolicina: " + std::to_string(kolicinaArtikla).substr(0, 5)).c_str(), CENTER_MODE);

                //iznosRacuna+=sviArtikli.at(i).cijena;
                ispis = "Ukupno: " + std::to_string(iznosRacuna).substr(0, 5)+ " KM";
                BSP_LCD_DisplayStringAt(0, 150,(uint8_t *)ispis.c_str(), LEFT_MODE);
                break;
            }
        }
        
        if (!nadjen) {
            skeniraniArtikal = Artikal();
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Nepostojeci artikal", CENTER_MODE);
        }
        
        BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
        BSP_LCD_FillRect(0, BSP_LCD_GetYSize()-40, BSP_LCD_GetXSize(), 40);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_DisplayStringAt(0, 215, (uint8_t *)"Zakljuci racun", CENTER_MODE);
    }
}

void mqtt_stigao_novi_artikal(MQTT::MessageData& md)
{
    if(trenutnoStanje==UNOS){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Unos artikala", CENTER_MODE);
        MQTT::Message& message = md.message;

        std::string payload =(char*)message.payload;
        payload.resize(message.payloadlen);
        std::string kod;
        std::string naziv;
        int cijena;
        int i=0;
        while(payload[i]!=',') kod+=payload[i++];
        i++;
        while(payload[i]!=',') naziv+=payload[i++];
        i++;
        cijena = stof(payload.substr(i));
        
        bool vecPostoji = false;
        
        for (char i = 0; i < sviArtikli.size(); ++i) {
            if (sviArtikli.at(i).barkod == kod) {
                vecPostoji = true;
                break;
            }
        }
        
        if (!vecPostoji) {
            Artikal noviArtikal;
            noviArtikal.barkod = kod;
            noviArtikal.naziv = naziv;
            noviArtikal.cijena = cijena;
            
            sviArtikli.push_back(noviArtikal);
        }
        
        BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Unijeli ste artikal:", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)naziv.c_str(), CENTER_MODE);
    }
}

void mqtt_stigao_barkod_za_brisanje(MQTT::MessageData& md)
{
    if(trenutnoStanje==BRISANJE){
        BSP_LCD_Clear(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Brisanje artikla", CENTER_MODE);
        MQTT::Message& message = md.message;

        std::string payload=(char*)message.payload;
        payload.resize(message.payloadlen);
        
        char offset = 0;
        bool postoji = false;
        
        for (; offset < sviArtikli.size(); ++offset) {
            if (sviArtikli.at(offset).barkod == payload) {
                postoji = true;
                break;
            }
        }
        
        if (postoji) {
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Obrisali ste artikal:", CENTER_MODE);
            BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)(*(sviArtikli.begin() + offset)).naziv.c_str(), CENTER_MODE);
            sviArtikli.erase(sviArtikli.begin() + offset);
        }
        else{
            BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Nepostojeci artikal", CENTER_MODE);
        }
    }

}

void pali_ledice() {
    
    char kolikoLedicaSkrozUpaliti = iznosRacuna / 20;
    if (kolikoLedicaSkrozUpaliti > 5) {
        kolikoLedicaSkrozUpaliti = 5;
    }
    
    for (char i = 0; i < kolikoLedicaSkrozUpaliti; ++i) {
        ledice[i] = 1.;
    }
    
    char sljedecaLedica = kolikoLedicaSkrozUpaliti;
    if (sljedecaLedica == 5) return;
    ledice[sljedecaLedica] = (iznosRacuna - kolikoLedicaSkrozUpaliti*20)/20.;
}

void gasi_ledice() {
    for (char i = 0; i < 5; ++i) {
        ledice[i] = 0.;
    }
}

int main() {
    sviArtikli.push_back(testArtikal);

    NetworkInterface *network;
    network = NetworkInterface::get_default_instance();

    if (!network) {
        return -1;
    }
    MQTTNetwork mqttNetwork(network);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "broker.hivemq.com";
    int port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\r\n", rc);
    else
        printf("Connected to broker!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = MQTT_CLIENT_NAME;
    data.username.cstring = "";
    data.password.cstring = "";
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\r\n", rc);

    if ((rc = client.subscribe(TEMAKUPOVINA, MQTT::QOS0, mqtt_stigao_skenirani_artikal)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMAKUPOVINA);


    if ((rc = client.subscribe(TEMAUNOS, MQTT::QOS0, mqtt_stigao_novi_artikal)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMAUNOS);

    if ((rc = client.subscribe(TEMABRISANJE, MQTT::QOS0, mqtt_stigao_barkod_za_brisanje)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    else
        printf("Subscribed to %s\r\n", TEMABRISANJE);
        
        
        
    BSP_LCD_Init();
    

    pocetno_stanje();
    nazadNaPocetno.fall(&pocetno_stanje);
    
    for (char i = 0; i < 5; ++i) {
        ledice[i].period_ms(200);
    }


    while (1) {
        rc = client.subscribe(TEMAKUPOVINA, MQTT::QOS0, mqtt_stigao_skenirani_artikal);
        rc = client.subscribe(TEMAUNOS, MQTT::QOS0, mqtt_stigao_novi_artikal);
        rc = client.subscribe(TEMABRISANJE, MQTT::QOS0, mqtt_stigao_barkod_za_brisanje);
        
        pali_ledice();
        
        if (promijenjenaKolicina != 0) {
            kupovina_stanje();
            promijenjenaKolicina = 0;
        }
        
        wait_ms(1);
    }
}
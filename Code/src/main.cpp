#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include "reseau_neurone.h"

#define TFT_DC D9
#define TFT_CS D10
#define TFT_RST D8

// constantes
const uint8_t X_LINE_1 = 50;
const uint8_t X_LINE_2 = 120;
const uint8_t X_LINE_3 = 190;
const uint8_t Y_LINE_1 = 115;
const uint8_t Y_LINE_2 = 185;
const uint8_t Y_LINE_3 = 255;

const uint8_t ADDR = 0x20;


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


int8_t tableau_jeu [10] = {0, 0, 0, 0,
                              0, 0, 0,
                              0, 0, 0};


enum mode_jeu {
  AUCUN,
  VSAMI,
  VSBOT
};

enum difficulte_bot {
  NONSELECTIONNE,
  FACILE = 50,
  MOYENNE = 30,
  DIFFICILE = 5,
};

enum gagnant {
  PERSONNE,
  MOI,
  AMI,
  BOT,
  EGALITE
};


/// @brief fonction pour lire le pin du MCP23018
/// @param[in] registre : registre sur lequel on lit (GPIOA ou GPIOB)
/// @return l'octet lu dans le registre
uint8_t lireMcp(char registre);

/// @brief fonction pour choisir le mode de jeu
/// @param[out] mode_jeu : mode de jeu séléctionné (contre un ami ou un bot)
void choixModeJeu(uint8_t & mode_jeu);

/// @brief fonction pour donner les coordonnées x et y d'un ponit dont on connait la position [1 à 9]
/// @param[in] pos : position dans la grille
/// @param[out] x : coordonnée x
/// @param[out] y : coordonnée y
void calculXY(uint8_t pos, uint8_t & x, uint8_t & y);

/// @brief fonction pour afficher une croix
/// @param[in] x : coordonnée x
/// @param[in] y : coordonnée y
/// @param[in] couleur : couleur de la croix
void dessinCroix (uint8_t x, uint8_t y, uint16_t couleur);

/// @brief fonction pour afficher les ronds et les croix à partir du tableau de jeu
void affichagePion();

/// @brief fonction pour valider la position où l'on place notre pion
/// @param[in] pos_select : position dans la grille
/// @param[in] joueur : soit moi (1) soit le joueur ennemi (2)
/// @param[out] nJoueur : ajoute plus 1 à la variable qui définit qui doit jouer
void validerPos(uint8_t pos_select, uint8_t joueur, uint8_t & nJoueur);

/// @brief fonction qui permet d'afficher un curseur clignotant sur ma position dans la grille
/// @param[in] pos_select : position dans la grille
/// @param[in] joueur : soit moi (1) soit le joueur ennemi (2)
void afficherCurseur(uint8_t pos_select, uint8_t joueur);

/// @brief fonction qui détecte si une personne a gagné ou s'il y a match nul
/// @param[in] mode_jeu : mode de jeu séléctionné (contre un ami ou un bot)
/// @return valeur qui permet de dire si la partie est fini et qui est le gagant
uint8_t finPartie(uint8_t mode_jeu);

/// @brief fonction pour déplacer mon pion dans la grille
/// @param[out] pos_select : position de mon pion dans la grille
void jeJoue(uint8_t & pos_select);

/// @brief fonction pour déplacer le pion de mon adversaire
/// @param[out] pos_select : position du pion ami dans la grille
void amiJoue(uint8_t & pos_select);

/// @brief fonction pour afficher la grille de jeu
void affichageGrille();

/// @brief fonction qui permet de retourner sur le menu principal
/// @param[out] pos_select : modification de la position séléctionné
/// @param[in] win : variable de fin de partie
/// @param[out] mode_jeu : mode de jeu séléctionné (contre un ami ou un bot)
void gestionRetour(uint8_t & pos_select, uint8_t win, uint8_t & mode_jeu);

/// @brief fonction qui gère toutes celles nécéssaires pour le mode de jeu contre un ami
/// @param[out] pos_select : position dans la grille de jeu
/// @param[out] mode_jeu : mode de jeu séléctionné (contre un ami ou un bot)
void jouerVsAmi(uint8_t & pos_select, uint8_t & mode_jeu);

/// @brief fonction pour afficher le menu de gestion de la difficulté du bot
/// @param[out] difficulte : niveau de difficulté du bot
void choixDifficulteBot(uint8_t & difficulte);

/// @brief fontion qui gère toutes celles nécéssaire pour le mode de jeu contre un bot
/// @param[out] pos_select : position dans la grille de jeu
/// @param mode_jeu : mode de jeu séléctionné (contre un ami ou contre un bot)
/// @param difficulte : niveau de difficulté du bot
void jouerVsBot(uint8_t & pos_select, uint8_t & mode_jeu, uint8_t difficulte);


void setup() {
  Wire.begin();
  Wire.beginTransmission(ADDR);
  Wire.write(0x0C);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.endTransmission();

  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH); // mettre la pin de reset à l'état haut pour évité de reset l'écran
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setTextSize(2);
}

void loop() {
  static uint8_t memo_mode_jeu = AUCUN;
  static uint8_t mode_jeu = AUCUN;
  static uint8_t position_select = 0;
  static uint8_t memoDifficulteBot = NONSELECTIONNE;
  static uint8_t difficulteBot = NONSELECTIONNE;

  if (mode_jeu == AUCUN){
    choixModeJeu(mode_jeu);
    difficulteBot = NONSELECTIONNE;
  }
  else if (mode_jeu == VSAMI){
    jouerVsAmi(position_select, mode_jeu);
  }
  else if (mode_jeu == VSBOT){
    if (difficulteBot == NONSELECTIONNE){
      choixDifficulteBot(difficulteBot);
    }
    else{
      jouerVsBot(position_select, mode_jeu, difficulteBot);
    }
  }
  
  if (memo_mode_jeu != mode_jeu){
    tft.fillScreen(ILI9341_WHITE);
    memo_mode_jeu = mode_jeu;
  }
  
  if ((memoDifficulteBot != difficulteBot) && (memoDifficulteBot == NONSELECTIONNE)){
    tft.fillScreen(ILI9341_WHITE);
  }
  memoDifficulteBot = difficulteBot;
}


uint8_t lireMcp(char registre){
  uint8_t ret = 0;

  Wire.beginTransmission(ADDR);
  Wire.write(0x12);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)2);
  uint8_t gpioa = Wire.read();
  uint8_t gpiob = Wire.read();
  if (registre == 'A'){
    ret = gpioa;
  }
  else if (registre == 'B'){
    ret = gpiob;
  }
  return (ret);
}

void choixModeJeu(uint8_t & mode_jeu){
  static uint8_t memo_btn_bas = (lireMcp('B') & 0x20)>>5;
  static uint8_t memo_btn_haut = (lireMcp('B') & 0x40)>>6;
  uint8_t act_btn_bas = (lireMcp('B') & 0x20)>>5; // lire la valeur du GPIO B 5
  uint8_t act_btn_haut = (lireMcp('B') & 0x40)>>6; // lire la valeur du GPIO B 6
  static uint8_t memo_btn_valid_g = (lireMcp('A') & 0x02)>>1;
  static uint8_t memo_btn_valid_d = (lireMcp('A') & 0x01);
  uint8_t act_btn_valid_g = (lireMcp('A') & 0x02)>>1; // lire la valeur du GPIO A 1
  uint8_t act_btn_valid_d = (lireMcp('A') & 0x01); // lire la valeur du GPIO A 0
  static uint8_t btn_select = 0;

  if ((btn_select == 0) && (memo_btn_bas != act_btn_bas) && (memo_btn_bas == 1)){
    btn_select = 1;
  }
  else if ((btn_select == 1) && (memo_btn_haut != act_btn_haut) && (memo_btn_haut == 1)){
    btn_select = 0;
  }
  else if (((memo_btn_valid_d != act_btn_valid_d) && (memo_btn_valid_d == 1)) || ((memo_btn_valid_g != act_btn_valid_g) && (memo_btn_valid_g == 1))){
    if (btn_select == 0){
      mode_jeu = VSAMI;
    }
    else if (btn_select == 1){
      mode_jeu = VSBOT;
    }
  }

  if (btn_select == 0){
    tft.drawRect(50, 75, 150, 50, ILI9341_GREEN);
    tft.drawRect(50, 150, 150, 50, ILI9341_BLACK);
  }
  else if (btn_select == 1){
    tft.drawRect(50, 75, 150, 50, ILI9341_BLACK);
    tft.drawRect(50, 150, 150, 50, ILI9341_GREEN);
  }

  tft.setCursor(55, 90);
  tft.print("JOUER VS AMI");
  tft.setCursor(55, 165);
  tft.print("JOUER VS BOT");

  memo_btn_bas = act_btn_bas;
  memo_btn_haut = act_btn_haut;
  memo_btn_valid_d = act_btn_valid_d;
  memo_btn_valid_g = act_btn_valid_g;
}

void calculXY(uint8_t pos, uint8_t & x, uint8_t & y){
  if (pos < 4){
    y = Y_LINE_1;
  }
  else if ((pos > 3) && (pos < 7)){
    y = Y_LINE_2;
  }
  else if (pos > 6){
    y = Y_LINE_3;
  }

  if ((pos == 1) || (pos == 4) || (pos == 7)){
    x = X_LINE_1;
  }
  else if ((pos == 2) || (pos == 5) || (pos == 8)){
    x = X_LINE_2;
  }
  else if ((pos == 3) || (pos == 6) || (pos == 9)){
    x = X_LINE_3;
  }
}

void dessinCroix (uint8_t x, uint8_t y, uint16_t couleur){
  tft.drawLine(x-10, y-10, x+10, y+10, couleur);
  tft.drawLine(x+10, y-10, x-10, y+10, couleur);
}

void affichagePion(){
  static uint8_t x = 0;
  static uint8_t y = 0;
  uint8_t i = 1;

  for(i=1; i<=9; i++){
    calculXY(i, x, y);
    if (tableau_jeu[i] == 1){
      dessinCroix(x, y, ILI9341_BLACK);
    }
    else if (tableau_jeu[i] == -1){
      tft.drawCircle(x, y, 20, ILI9341_BLACK);
    }
  }
}

void validerPos(uint8_t pos_select, uint8_t joueur, uint8_t & nJoueur){
  static uint8_t memo_btn_valid_g = (lireMcp('A') & 0x02)>>1;
  static uint8_t memo_btn_valid_d = (lireMcp('A') & 0x01);
  uint8_t act_btn_valid_g = (lireMcp('A') & 0x02)>>1; // lire la valeur du GPIO A 1
  uint8_t act_btn_valid_d = (lireMcp('A') & 0x01); // lire la valeur du GPIO A 0

  if (pos_select != 0){
    if (joueur == 1){
      if ((memo_btn_valid_d != act_btn_valid_d) && (memo_btn_valid_d == HIGH) && (tableau_jeu[pos_select] == 0)){
        tableau_jeu [pos_select] = 1;
        nJoueur ++;
      }
    }
    else if (joueur == 2){
      if ((memo_btn_valid_g != act_btn_valid_g) && (memo_btn_valid_g == HIGH) && (tableau_jeu[pos_select] == 0)){
        tableau_jeu [pos_select] = -1;
        nJoueur ++;
      }
    }
  }

  memo_btn_valid_d = act_btn_valid_d;
  memo_btn_valid_g = act_btn_valid_g;
}

void afficherCurseur(uint8_t pos_select, uint8_t joueur){
  static uint8_t memoPosSelect = pos_select;
  static uint32_t tpsPrec = millis();
  uint32_t tpsActu = millis();
  static uint8_t x = 0;
  static uint8_t y = 0;

  if (memoPosSelect != pos_select){
    if (joueur == 1){
      dessinCroix(x, y, ILI9341_WHITE);
    }
    else if (joueur == 2){
      tft.drawCircle(x, y, 20, ILI9341_WHITE);
    }
    memoPosSelect = pos_select;
    tpsPrec = tpsActu;
  }

  calculXY(pos_select, x, y);

  if ((tpsActu-tpsPrec <= 1000)){
    if (joueur == 1){
      dessinCroix(x, y, ILI9341_BLACK);
    }
    else if (joueur == 2){
      tft.drawCircle(x, y, 20, ILI9341_BLACK);
    }
  }
  else if ((tpsActu-tpsPrec >= 1000) && (tpsActu-tpsPrec <= 2000)){
    if (joueur == 1){
      dessinCroix(x, y, ILI9341_WHITE);
    }
    else if (joueur == 2){
      tft.drawCircle(x, y, 20, ILI9341_WHITE);
    }
  }
  else{
    tpsPrec = tpsActu;
  }
}

uint8_t finPartie(uint8_t mode_jeu){
  uint8_t win = PERSONNE;

  if ((tableau_jeu[1] == (int8_t)1) && (tableau_jeu[2] == (int8_t)1) && (tableau_jeu[3] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[1] == (int8_t)-1) && (tableau_jeu[2] == (int8_t)-1) && (tableau_jeu[3] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[4] == (int8_t)1) && (tableau_jeu[5] == (int8_t)1) && (tableau_jeu[6] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[4] == (int8_t)-1) && (tableau_jeu[5] == (int8_t)-1) && (tableau_jeu[6] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[7] == (int8_t)1) && (tableau_jeu[8] == (int8_t)1) && (tableau_jeu[9] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[7] == (int8_t)-1) && (tableau_jeu[8] == (int8_t)-1) && (tableau_jeu[9] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[1] == (int8_t)1) && (tableau_jeu[4] == (int8_t)1) && (tableau_jeu[7] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[1] == (int8_t)-1) && (tableau_jeu[4] == (int8_t)-1) && (tableau_jeu[7] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[2] == (int8_t)1) && (tableau_jeu[5] == (int8_t)1) && (tableau_jeu[8] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[2] == (int8_t)-1) && (tableau_jeu[5] == (int8_t)-1) && (tableau_jeu[8] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[3] == (int8_t)1) && (tableau_jeu[6] == (int8_t)1) && (tableau_jeu[9] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[3] == (int8_t)-1) && (tableau_jeu[6] == (int8_t)-1) && (tableau_jeu[9] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[1] == (int8_t)1) && (tableau_jeu[5] == (int8_t)1) && (tableau_jeu[9] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[1] == (int8_t)-1) && (tableau_jeu[5] == (int8_t)-1) && (tableau_jeu[9] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[3] == (int8_t)1) && (tableau_jeu[5] == (int8_t)1) && (tableau_jeu[7] == (int8_t)1)){
    win = MOI;
  }
  else if ((tableau_jeu[3] == (int8_t)-1) && (tableau_jeu[5] == (int8_t)-1) && (tableau_jeu[7] == (int8_t)-1)){
    win = 2;
  }
  else if ((tableau_jeu[1] != 0) && (tableau_jeu[2] != 0) && (tableau_jeu[3] != 0) &&
          (tableau_jeu[4] != 0) &&  (tableau_jeu[5] != 0) &&  (tableau_jeu[6] != 0) &&
          (tableau_jeu[7] != 0) &&  (tableau_jeu[8] != 0) &&  (tableau_jeu[9] != 0)) {
    win = EGALITE;
  }

  if (win == 2){
    if (mode_jeu == VSAMI){
      win = AMI;
    }
    else if (mode_jeu == VSBOT){
      win = BOT;
    }
  }

  return (win);
}

void jeJoue(uint8_t & pos_select){
  static uint8_t memo_btn_droit_d = (lireMcp('B') & 0x80)>>7;
  static uint8_t memo_btn_haut_d = (lireMcp('B') & 0x40)>>6;
  static uint8_t memo_btn_bas_d = (lireMcp('B') & 0x20)>>5;
  static uint8_t memo_btn_gauche_d = (lireMcp('B') & 0x10)>>4;
  uint8_t act_btn_droit_d = (lireMcp('B') & 0x80)>>7; // lire la valeur du GPIO B 7
  uint8_t act_btn_haut_d = (lireMcp('B') & 0x40)>>6; // lire la valeur du GPIO B 6
  uint8_t act_btn_bas_d = (lireMcp('B') & 0x20)>>5; // lire la valeur du GPIO B 5
  uint8_t act_btn_gauche_d = (lireMcp('B') & 0x10)>>4; // lire la valeur du GPIO B 4

  if ((pos_select == 2) && (memo_btn_haut_d != act_btn_haut_d) && (memo_btn_haut_d == HIGH)){
    pos_select = 0;
  }
  else if ((pos_select < 7) && (memo_btn_bas_d != act_btn_bas_d) && (memo_btn_bas_d == HIGH)){
    pos_select += 3;
  }
  else if ((pos_select > 3) && (memo_btn_haut_d != act_btn_haut_d) && (memo_btn_haut_d == HIGH)){
    pos_select -= 3;
  }
  else if ((pos_select != 1) && (pos_select != 4) && (pos_select != 7) && (memo_btn_gauche_d != act_btn_gauche_d) && (memo_btn_gauche_d == HIGH)){
    pos_select -= 1;
  }
  else if ((pos_select != 3) && (pos_select != 6) && (pos_select != 9) && (memo_btn_droit_d != act_btn_droit_d) && (memo_btn_droit_d == HIGH)){
    pos_select += 1;
  }

  memo_btn_bas_d = act_btn_bas_d;
  memo_btn_haut_d = act_btn_haut_d;
  memo_btn_gauche_d = act_btn_gauche_d;
  memo_btn_droit_d = act_btn_droit_d;
}

void amiJoue(uint8_t & pos_select){
  static uint8_t memo_btn_droit_g = (lireMcp('B') & 0x08)>>3;
  static uint8_t memo_btn_haut_g = (lireMcp('B') & 0x04)>>2;
  static uint8_t memo_btn_bas_g = (lireMcp('B') & 0x02)>>1;
  static uint8_t memo_btn_gauche_g = (lireMcp('B') & 0x01);
  uint8_t act_btn_droit_g = (lireMcp('B') & 0x08)>>3; // lire la valeur du GPIO B 3
  uint8_t act_btn_haut_g = (lireMcp('B') & 0x04)>>2; // lire la valeur du GPIO B 2
  uint8_t act_btn_bas_g = (lireMcp('B') & 0x02)>>1; // lire la valeur du GPIO B 1
  uint8_t act_btn_gauche_g = (lireMcp('B') & 0x01); // lire la valeur du GPIO B 0

  if ((pos_select == 2) && (memo_btn_haut_g != act_btn_haut_g) && (memo_btn_haut_g == HIGH)){
    pos_select = 0;
  }
  else if ((pos_select < 7) && (memo_btn_bas_g != act_btn_bas_g) && (memo_btn_bas_g == HIGH)){
    pos_select += 3;
  }
  else if ((pos_select > 3) && (memo_btn_haut_g != act_btn_haut_g) && (memo_btn_haut_g == HIGH)){
    pos_select -= 3;
  }
  else if ((pos_select != 1) && (pos_select != 4) && (pos_select != 7) && (memo_btn_gauche_g != act_btn_gauche_g) && (memo_btn_gauche_g == HIGH)){
    pos_select -= 1;
  }
  else if ((pos_select != 3) && (pos_select != 6) && (pos_select != 9) && (memo_btn_droit_g != act_btn_droit_g) && (memo_btn_droit_g == HIGH)){
    pos_select += 1;
  }

  memo_btn_bas_g = act_btn_bas_g;
  memo_btn_haut_g = act_btn_haut_g;
  memo_btn_gauche_g = act_btn_gauche_g;
  memo_btn_droit_g = act_btn_droit_g;
}

void affichageGrille(){
  tft.drawFastVLine(85, 80, 210, ILI9341_BLACK);
  tft.drawFastVLine(155, 80, 210, ILI9341_BLACK);
  tft.drawFastHLine(15, 150, 210, ILI9341_BLACK);
  tft.drawFastHLine(15, 220, 210, ILI9341_BLACK);
  tft.fillRect(110, 30, 20, 20, ILI9341_BLACK);
  tft.fillTriangle(105, 30, 120, 15, 135, 30, ILI9341_BLACK);
}

void gestionRetour(uint8_t & pos_select, uint8_t win, uint8_t & mode_jeu){
  static uint32_t tpsPrec = millis();
  uint32_t tpsActu = millis();
  static uint8_t memo_btn_bas_d = (lireMcp('B') & 0x20)>>5;
  static uint8_t memo_btn_bas_g = (lireMcp('B') & 0x02)>>1;
  uint8_t act_btn_bas_d = (lireMcp('B') & 0x20)>>5; // lire la valeur du GPIO B 5
  uint8_t act_btn_bas_g = (lireMcp('B') & 0x02)>>1; // lire la valeur du GPIO B 1
  static uint8_t memo_btn_valid_g = (lireMcp('A') & 0x02)>>1;
  static uint8_t memo_btn_valid_d = (lireMcp('A') & 0x01);
  uint8_t act_btn_valid_g = (lireMcp('A') & 0x02)>>1; // lire la valeur du GPIO A 1
  uint8_t act_btn_valid_d = (lireMcp('A') & 0x01); // lire la valeur du GPIO A 0
  uint8_t i = 1;

  if ((pos_select == 0) && (memo_btn_valid_d != act_btn_valid_d) && (memo_btn_valid_d == HIGH)){
    mode_jeu = AUCUN;
    for (i=1; i <= 9; i++){
      tableau_jeu[i] = 0;
    } 
  }
  else if ((pos_select == 0) && (memo_btn_valid_g != act_btn_valid_g) && (memo_btn_valid_g == HIGH)){
    mode_jeu = AUCUN;
    for (i=1; i <= 9; i++){
      tableau_jeu[i] = 0;
    } 
  }
  else if (pos_select == 0){
    if (((memo_btn_bas_d != act_btn_bas_d) && (memo_btn_bas_d == HIGH)) || ((memo_btn_bas_g != act_btn_bas_g) && (memo_btn_bas_g == HIGH))){
      pos_select = 2;
      tft.drawRect(95, 10, 50, 50, ILI9341_WHITE);
    }
    if (win == PERSONNE){
      tft.drawCircle(120, 115, 20, ILI9341_WHITE);
      tft.drawLine(110, 105, 130, 125, ILI9341_WHITE);
      tft.drawLine(130, 105, 110, 125, ILI9341_WHITE);
    }
  }

  if (tpsActu-tpsPrec <= 250){
    if (pos_select == 0){
      tft.drawRect(95, 10, 50, 50, ILI9341_GREEN);
    }
  }
  else if ((tpsActu-tpsPrec >= 250) && (tpsActu-tpsPrec <= 500)){
    if(pos_select == 0){
      tft.drawRect(95, 10, 50, 50, ILI9341_WHITE);
    }
  }
  else{
    tpsPrec = tpsActu;
  }

  memo_btn_bas_d = act_btn_bas_d;
  memo_btn_valid_d = act_btn_valid_d;
  memo_btn_valid_g = act_btn_valid_g;
}

void jouerVsAmi(uint8_t & pos_select, uint8_t & mode_jeu){
  static uint8_t joueurJoue = 1;
  static uint8_t win = PERSONNE;

  affichageGrille();
  tft.setCursor(37, 60);
  affichagePion();

  win = finPartie(VSAMI);

  gestionRetour(pos_select, win, mode_jeu);

  if ((pos_select != 0) && (win != PERSONNE)){
    tft.setCursor(10, 60);
    pos_select = 0;
    if (win == MOI){
      tft.print("joueur droit gagne");
    }
    else if (win == AMI){
      tft.print("joueur gauche gagne");
    }
    else if (win == EGALITE){
      tft.print("il y a match nul");
    }
  }
  else if (pos_select != 0){
    if ((joueurJoue%2) == 0){
      tft.print("joueur droit > ");
      jeJoue(pos_select);
      afficherCurseur(pos_select, 1);
      validerPos(pos_select, 1, joueurJoue);
    }
    else{
      tft.print("joueur gauche <");
      amiJoue(pos_select);
      afficherCurseur(pos_select, 2);
      validerPos(pos_select, 2, joueurJoue);
    }
  }
}

void choixDifficulteBot(uint8_t & difficulte){
  static uint8_t memo_btn_bas = (lireMcp('B') & 0x20)>>5;
  static uint8_t memo_btn_haut = (lireMcp('B') & 0x40)>>6;
  uint8_t act_btn_bas = (lireMcp('B') & 0x20)>>5; // lire la valeur du GPIO B 5
  uint8_t act_btn_haut = (lireMcp('B') & 0x40)>>6; // lire la valeur du GPIO B 6
  static uint8_t memo_btn_valid_g = (lireMcp('A') & 0x02)>>1;
  static uint8_t memo_btn_valid_d = (lireMcp('A') & 0x01);
  uint8_t act_btn_valid_g = (lireMcp('A') & 0x02)>>1; // lire la valeur du GPIO A 1
  uint8_t act_btn_valid_d = (lireMcp('A') & 0x01); // lire la valeur du GPIO A 0
  static uint8_t btn_select = 0;  

  if ((btn_select != 2) && (memo_btn_bas != act_btn_bas) && (memo_btn_bas == 1)){
    btn_select += 1;
  }
  else if ((btn_select != 0) && (memo_btn_haut != act_btn_haut) && (memo_btn_haut == 1)){
    btn_select -= 1;
  }
  else if (((memo_btn_valid_d != act_btn_valid_d) && (memo_btn_valid_d == 1)) || ((memo_btn_valid_g != act_btn_valid_g) && (memo_btn_valid_g == 1))){
    if (btn_select == 0){
      difficulte = FACILE;
    }
    else if (btn_select == 1){
      difficulte = MOYENNE;
    }
    else if (btn_select == 2){
      difficulte = DIFFICILE;
    }
  }

  if (btn_select == 0){
    tft.drawRect(50, 50, 150, 50, ILI9341_GREEN);
    tft.drawRect(50, 125, 150, 50, ILI9341_BLACK);
    tft.drawRect(50, 200, 150, 50, ILI9341_BLACK);
  }
  else if (btn_select == 1){
    tft.drawRect(50, 50, 150, 50, ILI9341_BLACK);
    tft.drawRect(50, 125, 150, 50, ILI9341_GREEN);
    tft.drawRect(50, 200, 150, 50, ILI9341_BLACK);
  }
  else if (btn_select == 2){
    tft.drawRect(50, 50, 150, 50, ILI9341_BLACK);
    tft.drawRect(50, 125, 150, 50, ILI9341_BLACK);
    tft.drawRect(50, 200, 150, 50, ILI9341_GREEN);
  }

  tft.setCursor(90, 65);
  tft.print("FACILE");
  tft.setCursor(95, 140);
  tft.print("MOYEN");
  tft.setCursor(70, 215);
  tft.print("DIFFICILE");

  memo_btn_bas = act_btn_bas;
  memo_btn_haut = act_btn_haut;
  memo_btn_valid_d = act_btn_valid_d;
  memo_btn_valid_g = act_btn_valid_g;
}

void jouerVsBot(uint8_t & pos_select, uint8_t & mode_jeu, uint8_t difficulte){
  static uint8_t joueurJoue = 2;
  static uint8_t win = PERSONNE;

  affichageGrille();
  tft.setCursor(100, 60);
  affichagePion();

  win = finPartie(VSBOT);

  gestionRetour(pos_select, win, mode_jeu);

  if ((pos_select != 0) && (win != PERSONNE)){
    tft.setCursor(40, 60);
    pos_select = 0;
    joueurJoue = 2;
    if (win == MOI){
      tft.print("vous avez gagne");
    }
    else if (win == BOT){
      tft.print("le bot a gagne");
    }
    else if (win == EGALITE){
      tft.print("il y a match nul");
    }
  }
  else if (pos_select != 0){
    if ((joueurJoue%2) == 0){
      tft.print("vous");
      jeJoue(pos_select);
      afficherCurseur(pos_select, 1);
      validerPos(pos_select, 1, joueurJoue);
    }
    else{
      uint8_t coupBot;
      tft.print("bot ");
      coupBot = nn_move_level(tableau_jeu, difficulte);
      if (coupBot != 0){
        tableau_jeu[coupBot] = -1;
        joueurJoue += 1;
      }
    }
  }
}
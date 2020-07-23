/******************************************************************************/
/*                                                                            */
/*  Elektrotechnik und Informationstechnik (HAW)                              */
/*  Konzeption und Umsetzung eines Demonstrators für Fehlerangriff            */
/*  Implementierung des RSA-CRT-1024 auf Microkontrolle                       */
/*  Autor: Minh Tung Bui, Marcel Delissen                                                      */
/*  Datum: 16.06.2020                                                         */
/******************************************************************************/
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flint.h"
#include <time.h>


#define LED2 25
#define TRIGGER 8           // Pin P.1.10
#define SIMULATION 0        // 0 oder 1
#define ANZAHL_BIT 32     // 32, 1024, 256, 512

// Globale Variable
//--------------------------------------------------------------------------------------------------------//
CLINT p_primzahl;
CLINT q_primzahl;
CLINT n_schluessel;
CLINT e_schluessel;
CLINT d_schluessel;

CLINT klar_nachricht;
CLINT rsa_verschluesseln;
CLINT rsa_entschluesseln;
CLINT signatur;
CLINT rsa_crt_entschluesseln;
//--------------------------------------------------------------------------------------------------------//

// Benötige Funktionen
//--------------------------------------------------------------------------------------------------------//
void key_nachricht_eingabe(CLINT  p_primzahl, CLINT q_primzahl, CLINT n_schluessel, CLINT e_schluessel, CLINT d_schluessel, CLINT klar_nachricht);
void q_m_algorithmus(CLINT x, CLINT k, CLINT n, CLINT result);
void rsa_crt_berechnung(CLINT p, CLINT q, CLINT n, CLINT d, CLINT m, CLINT sig);
void Rekonstruktion(CLINT n, CLINT e, CLINT false_sig, CLINT m);
//--------------------------------------------------------------------------------------------------------//


/******************************************************************************/
/*                                                                            */
/*  Funktion:  zur Eingabe des Schlüssel und Nachricht                        */
/*  Syntax:    void key_nachricht_eingabe(CLINT  p, CLINT q, CLINT n, CLINT e,*/
/*                      CLINT d, CLINT nachricht)     */
/*  Eingabe:   p,q,e,d (Schlüssel), nachricht (nachricht zum verschlüsseln)   */
/*  Ausgabe:   n (öffentlicher Schlüsseln),                   */
/*  Rueckgabe: null                               */
/*                                        */
/******************************************************************************/
void key_nachricht_eingabe(CLINT  p, CLINT q, CLINT n, CLINT e, CLINT d, CLINT nachricht)
{
  char* p_string;
  char* q_string;
  char* e_string;
  char* d_string;
  char* nachricht_string;

  // define the key
 if(ANZAHL_BIT == 1024)
 {
  p_string = "12355447201996786223067845453089966155621529271987222799944585111546270800751840420291657666412823976768984203289835886682223739447439167720310045412751343";
  q_string = "7964425044439495923757589331704696128150003049220525388097712928890862029392562595568033481348699709432989762039048403304516632936475468458303019508597377";
  e_string = "65537";
  d_string = "58036112250682224834609318837259461007715489847295721103231439416162020536804016599384512414167238220088768259793424232018681277966877157132821579731484061495793938596152416549039074678298306184628891517674155481943952508285804132725503524179321307189118313087839129280922257414197819978344781120922280571905";
  nachricht_string = "30";
 }
 else if(ANZAHL_BIT == 32)
 {
  p_string = "57287";
  q_string = "35099";

  e_string = "17";
  d_string = "354816005";
  nachricht_string = "10";
 }
 else if(ANZAHL_BIT == 56)
 {
  p_string = "179747107";
  q_string = "145564691";

  e_string = "17";
  d_string = "3078215501563193";
  nachricht_string = "10";
 }
 else if(ANZAHL_BIT == 256)
 {
  p_string = "270151268997351570427300498619178698791";
  q_string = "173992259560136077954468563215860193761";
  e_string = "17";
  d_string = "16589728135019060126979788698714626009056588370954210116464117109249674429553";
  nachricht_string = "10";
 }
  
  // Konvertierung des Zeichenkette
  str2clint_l(p, p_string, 10);
  str2clint_l(q, q_string, 10);
  str2clint_l(e, e_string, 10);
  str2clint_l(d, d_string, 10);
  str2clint_l(nachricht, nachricht_string, 10);
  
  // öffentlicher Schlüssel n berechnen
  mul_l(p, q, n);
}

//--------------------------------------------------------------------------------------------------------//
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Berechnung zum Quadrat-Multiplikation-Algorithmus              */
/*  Syntax:    void q_m_algorithmus(CLINT x, CLINT k, CLINT n, CLINT result)  */
/*  Eingabe:   x (Base), k(Exponent) , n (Divisor)                            */
/*  Ausgabe:   result (Rest)                          */
/*  Rueckgabe: null                               */
/*                                        */
/******************************************************************************/
void q_m_algorithmus(CLINT x, CLINT k, CLINT n, CLINT result)
{
  char *k_string = xclint2str_l(k, 2, 0);

  str2clint_l(result, "1", 10);
  for (int i = 0; i < (int)strlen(k_string); i++)
  {
    mmul_l(result, result, result, n);
    if (k_string[i] == '1')
    {
      mmul_l(result, x, result, n);
    }
  }

}
//--------------------------------------------------------------------------------------------------------//

/******************************************************************************/
/*                                                                            */
/*  Funktion:  Berechnung zum RSA_CRT_Algorithmus                             */
/*  Syntax:    void rsa_crt_berechnung(CLINT p, CLINT q, CLINT n, CLINT d,    */
/*           CLINT m, CLINT sig);                                             */
/*  Eingabe:   Schlüssel (p,q,n,d) , m (Nachricht)                            */
/*  Ausgabe:   sig (Signatur)                                                 */
/*  Rueckgabe: null                                                           */
/*                                        */
/******************************************************************************/
void rsa_crt_berechnung(CLINT p, CLINT q, CLINT n, CLINT d, CLINT m, CLINT sig)
{
  int vorzeichen_u;
  int vorzeichen_v;
    
  CLINT p_1;            // Ergebnis: p-1
  CLINT q_1;            // Ergebnis: q-1
  CLINT d_p;            // Ergebnis: dp = d mod(p-1)
  CLINT d_q;            // Ergebnis: dq = d mod(q-1)
  CLINT ggT;            // Größter gemeinsamer Teiler von p und q
  CLINT u;              // Faktor u
  CLINT v;              // Faktor v
  
  CLINT sig_p;          // Ergebnis sig_p = m^(dp) mod p 
  CLINT sig_q;          // Ergebnis sig_q = m^(dq) mod q 
  
  CLINT p_sigq;         // Ergebnis: p*sig_q
  CLINT u_mod_n;        // Ergebnis: u mod n
  CLINT u_p_sigq_mod_n; // Ergebnis: u*p*sigq mod n
  CLINT sub_n_u;        // Ergebnis: n-u
  
  CLINT q_sigp;         // Ergebnis: q*sig_p
  CLINT v_mod_n;        // Ergebnis: v mod n
  CLINT v_q_sigp_mod_n; // Ergebnis: v*q*sigp mod n
  CLINT sub_n_v;        // Ergebnis: n-v
  
  CLINT sum;            // Ergebnis: u*p*sigq + v*q*sigp

  //
  cpy_l(p_1, p);        // p_1, q_1 berechnnen
  dec_l(p_1);
  cpy_l(q_1, q);
  dec_l(q_1);

  mod_l(d, p_1, d_p);   // dp berechnen
  mod_l(d, q_1, d_q);   // dq berechnen

  xgcd_l(p, q, ggT, u, &vorzeichen_u, v, &vorzeichen_v);
  
  digitalWrite(LED2, HIGH); 
  digitalWrite(TRIGGER, HIGH);

  // Test false_sig
  if(SIMULATION == 1)
  {
    char* false_sig_p_string;
    false_sig_p_string = "0";
    str2clint_l(sig_p ,false_sig_p_string, 10);
  }
  else if(SIMULATION == 0)
  {
    q_m_algorithmus(m, d_p, p, sig_p);  //sig_p sig_q berechnen
  }
  
  digitalWrite(LED2, LOW);
  //digitalWrite(TRIGGER, LOW);


  digitalWrite(LED2, HIGH); 
  digitalWrite(TRIGGER, HIGH);
  q_m_algorithmus(m, d_q, q, sig_q);
  digitalWrite(LED2, LOW);
  //digitalWrite(TRIGGER, LOW);
  
  // Betrachtung: ob u größer oder kleiner als 0 ist?
  if (vorzeichen_u >= 0)
  {
    mod_l(u, n, u_mod_n);
  }
  
  else if (vorzeichen_u < 0)
  {
    sub_l(n, u, sub_n_u);
    mod_l(sub_n_u, n, u_mod_n);
  }

  // Betrachtung: ob v größer oder kleiner als 0 ist?
  if (vorzeichen_v >= 0)
  {
    mod_l(v, n, v_mod_n);
  }
  else if (vorzeichen_v < 0)
  {
    sub_l(n, v, sub_n_v);
    mod_l(sub_n_v, n, v_mod_n);
  }

  // Berechnung u*p*sigq
  digitalWrite(LED2, HIGH); 
  digitalWrite(TRIGGER, HIGH);
  mmul_l(p, sig_q, p_sigq, n);
  mmul_l(p_sigq, u_mod_n, u_p_sigq_mod_n, n);
  digitalWrite(LED2, LOW);
  //digitalWrite(TRIGGER, LOW);

  // Berechnung v*q*sigp
  digitalWrite(LED2, HIGH); 
  digitalWrite(TRIGGER, HIGH);
  mmul_l(q, sig_p, q_sigp, n);
  mmul_l(q_sigp, v_mod_n, v_q_sigp_mod_n, n);
  digitalWrite(LED2, LOW);
  //digitalWrite(TRIGGER, LOW);

  // Berechnung u*p*sigq + v*q*sigp
  digitalWrite(LED2, HIGH); 
  digitalWrite(TRIGGER, HIGH);
  add_l(u_p_sigq_mod_n, v_q_sigp_mod_n, sum);
  digitalWrite(LED2, LOW);
  //digitalWrite(TRIGGER, LOW);

  // Berechnung sig = (u*p*sigq + v*q*sigp) mod n
  digitalWrite(LED2, HIGH); 
  digitalWrite(TRIGGER, HIGH);
  mod_l(sum, n, sig);
  digitalWrite(LED2, LOW);
  //digitalWrite(TRIGGER, LOW);
}
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Rekonstruktion des privaten Schlüssels d                       */
/*  Syntax:    void Rekonstruktion(CLINT n, CLINT e, CLINT false_sig, CLINT m)*/                                            
/*  Eingabe:   Schlüssel (n,e) , m (Nachricht), false_sig                      */
/*  Ausgabe:   privaten Schlüssel d                                             */
/*  Rueckgabe: null                                                           */
/*                                        */
/******************************************************************************/
void Rekonstruktion(CLINT n, CLINT e, CLINT false_sig, CLINT m)
{
  CLINT d;

  CLINT u_clint;            // Faktor u
  CLINT v_clint;            // Faktor v
  CLINT m_mod_n;

  CLINT false_sig_e;
  CLINT m_false_sig_e;        // m-false_sig ^ e
  CLINT m_false_sig_mod_n;      // (m-false_sig) mod n
  CLINT Primzahl1;          // (Primzahl 1) es kann p oder q sein
  CLINT Primzahl2;          // (Primzahl 2) es kann p oder q sein
  CLINT Primzahl1_dec;
  CLINT Primzahl2_dec;
  CLINT phi;
  CLINT Rest;
  CLINT x;

  int vorzeichen_u;
  int vorzeichen_v;

  mod_l(m, n, m_mod_n);
  q_m_algorithmus(false_sig, e, n, false_sig_e);


  int vergleich = cmp_l(m_mod_n, false_sig_e);

  // Wenn m_mod_n größer als false_sig_e
  if (vergleich == 1)
  {
    sub_l(m_mod_n, false_sig_e, m_false_sig_e);
    mod_l(m_false_sig_e, n, m_false_sig_mod_n);
    
  }

  // Wenn m_mod_n kleiner als false_sig_e
  else if (vergleich == -1)
  { 
    CLINT Sub1;
    CLINT Sub2;
    CLINT Result;

    sub_l(false_sig_e, m_mod_n, Sub2);
    cpy_l(Sub1, n);

    while (1)
    {
      add_l(Sub1, n, Sub1);
      int loop = cmp_l(Sub1, Sub2);
      if (loop == 1)
        break;
    }

    sub_l(Sub1, Sub2, Result);
    mod_l(Result, n, m_false_sig_mod_n);  
  }
  
  // berechnen Primzahl1 aus n und (m-false_sig^e) mod n
  xgcd_l(n, m_false_sig_mod_n, Primzahl1, u_clint, &vorzeichen_u, v_clint, &vorzeichen_v);
  

  // berechnet Primzahl2
  div_l(n, Primzahl1, Primzahl2, Rest);
  
  // phi berechnen
  cpy_l(Primzahl1_dec, Primzahl1);
  cpy_l(Primzahl2_dec, Primzahl2);
  dec_l(Primzahl1_dec);
  dec_l(Primzahl2_dec);
  mul_l(Primzahl1_dec, Primzahl2_dec, phi);

  // d berechnen
  inv_l(e, phi, x, d);

  Serial.println("(Rekonstruktion) Primzahl 1:");
  Serial.println(xclint2str_l(Primzahl1, 10, 0));
  Serial.println("(Rekonstruktion) Primzahl 2:");
  Serial.println(xclint2str_l(Primzahl2, 10, 0));
  Serial.println("(Rekonstruktion) Privater Schlueeseln d :");
  Serial.println(xclint2str_l(d, 10, 0));
}


//--------------------------------------------------------------------------------------------------------//
// Die Setup-Funktion wird einmal ausgeführt, wenn Sie Reset drücken oder die Karte mit Strom versorgen
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(TRIGGER, OUTPUT);

  // init key
  key_nachricht_eingabe(p_primzahl, q_primzahl, n_schluessel, e_schluessel, d_schluessel, klar_nachricht);
}
//--------------------------------------------------------------------------------------------------------//


// Die Schleifenfunktion läuft immer wieder für immer
void loop() {

  //while(1)
  //{
     
      // c = m^e mod n
     // q_m_algorithmus(klar_nachricht, e_schluessel, n_schluessel, rsa_verschluesseln);

      // signatur erstellen
     rsa_crt_berechnung(p_primzahl, q_primzahl, n_schluessel, d_schluessel, klar_nachricht, signatur);

    
      // LED einschalten (HIGH ist der Spannungspegel)   
     // delay(1000);
     // m = c^d mod n
     // q_m_algorithmus(rsa_verschluesseln, d_schluessel, n_schluessel, rsa_entschluesseln);
     // Signaltur entschlüsseln
     q_m_algorithmus(signatur, e_schluessel, n_schluessel, rsa_crt_entschluesseln);

     digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW

     /*
     Serial.println("-----Implementierung RSA−CRT−1024-----");
     Serial.println("Primzahl p:");
     Serial.println(xclint2str_l(p_primzahl, 10, 0));
     Serial.println("Primzahl q:");
     Serial.println(xclint2str_l(q_primzahl, 10, 0));

     Serial.println("Key n:");
     Serial.println(xclint2str_l(n_schluessel, 10, 0));
     Serial.println("Key e:");
     Serial.println(xclint2str_l(e_schluessel, 10, 0));
     Serial.println("Key d:");
     Serial.println(xclint2str_l(d_schluessel, 10, 0));     
     */ 
     Serial.println("---------------Nachricht--------------");
     Serial.println(xclint2str_l(klar_nachricht, 10, 0));
     
     /*
     Serial.println("--------------------------------------");
     Serial.println("verschluecsselte Nachricht (c):");
     Serial.println(xclint2str_l(rsa_verschluesseln, 10, 0));
     Serial.println("entschluecsselte Nachricht (m):");
     Serial.println(xclint2str_l(rsa_entschluesseln, 10, 0));
     */
     Serial.println("--------------------------------------");
     Serial.println("Signatur sig:");
     Serial.println(xclint2str_l(signatur, 10, 0));
     Serial.println("entschluecsselte Nachricht (m):");
     Serial.println(xclint2str_l(rsa_crt_entschluesseln, 10, 0)); 
     Serial.println("--------------------------------------\n\n");
     digitalWrite(LED_BUILTIN, LOW);
     delay(1000);

     int zahl = equ_l(rsa_crt_entschluesseln,klar_nachricht);
     if(zahl == 0)
     {
      Serial.println("Fehler: Falsche Signatur");
      Serial.println("Falsche Signatur ist:");
      Serial.println(xclint2str_l(signatur, 10, 0));
      Rekonstruktion(n_schluessel, e_schluessel, signatur, klar_nachricht); 
      delay(10000);
     }

      // Vergleich entschlüsseltes Endergebnis mit vorgegebenen Nachricht
   //  if(rsa_crt_entschluesseln != klar_nachricht)
   //  break;
 // }
 // digitalWrite(LED_BUILTIN, HIGH);
     
}

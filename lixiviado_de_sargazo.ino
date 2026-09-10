// =================================================================
// VALIDACIÓN CONTRA DATASET EXPERIMENTAL Y DE LITERATURA DE CAMPO
// Proyecto: Sistema de Monitoreo de Calidad de Agua (Sargazo)
// Objetivo: Verificación de Convergencia en Escenarios Reales
// =================================================================

struct CasoCampo {
  String id_caso;
  String referencia;
  float dt;
  float od;
  float nh4;
  float no3;
  float turb;
};

// Dataset de Validación: 15 Escenarios Físico-Químicos Extraídos de Literatura/Campo
CasoCampo datasetCampo[15] = {
  // --- CONDICIONES CONTROL / AGUA LIMPIA (Arrecife no impactado / Puerto Morelos - Mahahual) ---
  {"EXP_VERDE_1", "van Tussenbroek (2017)",  0.1, 8.2, 0.1, 0.3, 0.2},  
  {"EXP_VERDE_2", "Cabanillas-Teran (2022)", 0.0, 7.5, 0.3, 0.5, 0.3},  
  {"EXP_VERDE_3", "Rodriguez-Mtz (2020)",   0.2, 8.0, 0.2, 0.2, 0.1},  
  {"EXP_VERDE_4", "van Tussenbroek (2017)",  0.3, 6.8, 0.5, 0.8, 0.4},  
  {"EXP_VERDE_5", "Cabanillas-Teran (2022)", 0.1, 7.9, 0.2, 0.4, 0.2},  

  // --- ARRIBAZÓN RECIENTE / PLUMA DILUIDA (Alertas Moderadas / Eventos Iniciales) ---
  {"EXP_AMAR_1",  "Cabanillas-Teran (2022)", 0.8, 4.8, 2.1, 2.8, 1.1},  
  {"EXP_AMAR_2",  "van Tussenbroek (2017)",  0.9, 4.2, 1.8, 2.0, 0.9},  
  {"EXP_AMAR_3",  "Rodriguez-Mtz (2020)",   0.6, 5.5, 3.2, 1.8, 1.4},  
  {"EXP_AMAR_4",  "Cabanillas-Teran (2022)", 0.7, 5.0, 1.2, 4.5, 1.0},  
  {"EXP_AMAR_5",  "van Tussenbroek (2017)",  1.0, 3.8, 2.5, 3.1, 1.8},  

  // --- MAREA MARRÓN / LIXIVIADO CONCENTRADO (Anoxia Severa / Mahahual - Xcalak) ---
  {"EXP_ROJO_1",  "van Tussenbroek (2017)",  1.5, 0.5, 18.2, 8.5, 4.2}, 
  {"EXP_ROJO_2",  "Cabanillas-Teran (2022)", 1.8, 1.2, 15.0, 9.2, 3.9}, 
  {"EXP_ROJO_3",  "Rodriguez-Mtz (2020)",   1.2, 2.0, 22.1, 6.0, 4.5}, 
  {"EXP_ROJO_4",  "van Tussenbroek (2017)",  1.4, 0.2, 12.8, 7.1, 4.8}, 
  {"EXP_ROJO_5",  "Cabanillas-Teran (2022)", 1.6, 1.5, 19.5, 11.0, 4.0}  
};

void setup() {
  Serial.begin(115200);
  delay(2000);

  // =================================================================
  // IMPRESIÓN DEL DATASET EXPERIMENTAL Y VALIDACIÓN DIFUSA
  // =================================================================
  Serial.println("\n========================================================================================================================");
  Serial.println("TABLA DE RESULTADOS: EVALUACION DEL DATASET EXPERIMENTAL Y DE CAMPO (LITERATURA)");
  Serial.println("========================================================================================================================");
  Serial.println("ID_Caso\t\tReferencia_Fuente\t\tDT\tOD\tNH4\tNO3\tTURB\tP_Simp\tFuzzy(Sef)\tTRIX\tI_Mat");
  Serial.println("------------------------------------------------------------------------------------------------------------------------");

  for (int i = 0; i < 15; i++) {
    float p_simp  = calcularProbabilidadReal(datasetCampo[i].dt, datasetCampo[i].od, datasetCampo[i].nh4, datasetCampo[i].no3, datasetCampo[i].turb);
    float f_fuzzy = ejecutarInferenciaFuzzy(datasetCampo[i].dt, datasetCampo[i].od, datasetCampo[i].nh4, datasetCampo[i].no3, datasetCampo[i].turb);
    float t_trix  = calcularIndiceTRIX(datasetCampo[i].dt, datasetCampo[i].od, datasetCampo[i].nh4, datasetCampo[i].no3);
    float i_mat   = calcularIndiceMatematico(datasetCampo[i].dt, datasetCampo[i].od, datasetCampo[i].nh4, datasetCampo[i].no3, datasetCampo[i].turb);

    Serial.print(datasetCampo[i].id_caso);       Serial.print("\t");
    Serial.print(datasetCampo[i].referencia);    Serial.print("\t");
    Serial.print(datasetCampo[i].dt, 1);         Serial.print("\t");
    Serial.print(datasetCampo[i].od, 1);         Serial.print("\t");
    Serial.print(datasetCampo[i].nh4, 1);        Serial.print("\t");
    Serial.print(datasetCampo[i].no3, 1);        Serial.print("\t");
    Serial.print(datasetCampo[i].turb, 1);       Serial.print("\t");
    Serial.print(p_simp, 0);                    Serial.print("\t");
    Serial.print(f_fuzzy, 1);                   Serial.print("\t\t");
    Serial.print(t_trix, 1);                    Serial.print("\t");
    Serial.println(i_mat, 1);
  }
  Serial.println("========================================================================================================================");
}

void loop() {
  // Sin ciclo continuo
}

// =================================================================
// SECCIÓN 1: OPERADORES DE MEMBRESÍA
// =================================================================
float trapmf(float x, float a, float b, float c, float d) {
  if (x <= a || x >= d) return 0.0;
  if (x >= b && x <= c) return 1.0;
  if (x > a && x < b) return (x - a) / (b - a);
  if (x > c && x < d) return (d - x) / (d - c);
  return 0.0;
}

// =================================================================
// SECCIÓN 2: MOTOR DE INFERENCIA FUZZY (SEMÁFORO DE 20 REGLAS)
// =================================================================
float ejecutarInferenciaFuzzy(float dt, float od, float nh4, float no3, float turb) {
  float dt_normal   = trapmf(dt, 0.0, 0.0, 0.6, 1.2);
  float dt_anomalia = trapmf(dt, 0.6, 1.2, 6.0, 6.0);
  float od_bajo     = trapmf(od, 0.0, 0.0, 2.5, 4.0);
  float od_medio    = trapmf(od, 3.0, 4.5, 5.5, 7.0);
  float od_alto     = trapmf(od, 6.0, 7.5, 10.0, 10.0);
  float nh4_bajo    = trapmf(nh4, 0.0, 0.0, 1.5, 3.0);
  float nh4_elevado = trapmf(nh4, 1.5, 8.0, 12.0, 18.0);
  float nh4_extremo = trapmf(nh4, 15.0, 20.0, 25.0, 25.0);
  float no3_bajo    = trapmf(no3, 0.0, 0.0, 1.5, 4.0);
  float no3_alto    = trapmf(no3, 1.5, 6.0, 15.0, 15.0);
  float turb_cristalina = trapmf(turb, 0.0, 0.0, 0.3, 0.7);
  float turb_baja       = trapmf(turb, 0.4, 1.0, 1.4, 2.2);
  float turb_residuos   = trapmf(turb, 1.4, 3.0, 5.0, 5.0);

  float w_verde = 0.0, w_amarillo = 0.0, w_rojo = 0.0;

  // REGLAS ROJO
  w_rojo = fmax(w_rojo, nh4_extremo); 
  w_rojo = fmax(w_rojo, fmin(od_bajo, turb_residuos)); 
  w_rojo = fmax(w_rojo, fmin(dt_anomalia, fmin(od_bajo, turb_baja))); 
  w_rojo = fmax(w_rojo, fmin(od_bajo, nh4_elevado)); 
  w_rojo = fmax(w_rojo, fmin(od_medio, fmin(nh4_elevado, no3_alto))); 
  w_rojo = fmax(w_rojo, fmin(od_medio, fmin(nh4_elevado, turb_residuos))); 
  w_rojo = fmax(w_rojo, fmin(od_alto, fmin(nh4_elevado, turb_residuos))); 

  // REGLAS VERDE
  w_verde = fmax(w_verde, fmin(od_alto, fmin(nh4_bajo, turb_cristalina))); 
  w_verde = fmax(w_verde, fmin(dt_normal, fmin(od_medio, fmin(nh4_bajo, fmin(no3_bajo, turb_cristalina))))); 
  w_verde = fmax(w_verde, fmin(dt_normal, fmin(od_medio, fmin(nh4_bajo, turb_baja)))); 
  w_verde = fmax(w_verde, fmin(od_alto, fmin(nh4_bajo, turb_baja))); 
  w_verde = fmax(w_verde, fmin(dt_anomalia, fmin(od_medio, fmin(nh4_bajo, fmin(no3_bajo, turb_cristalina))))); 

  // REGLAS AMARILLO
  w_amarillo = fmax(w_amarillo, fmin(od_alto, fmin(nh4_elevado, turb_baja))); 
  w_amarillo = fmax(w_amarillo, fmin(od_medio, fmin(nh4_bajo, turb_baja))); 
  w_amarillo = fmax(w_amarillo, fmin(dt_normal, fmin(od_bajo, fmin(nh4_bajo, turb_baja)))); 
  w_amarillo = fmax(w_amarillo, fmin(od_alto, fmin(nh4_elevado, fmin(no3_alto, turb_baja)))); 
  w_amarillo = fmax(w_amarillo, fmin(od_medio, fmin(nh4_bajo, fmin(no3_alto, turb_baja)))); 
  w_amarillo = fmax(w_amarillo, fmin(od_alto, fmin(nh4_bajo, fmin(no3_alto, fmax(turb_cristalina, turb_baja))))); 
  w_amarillo = fmax(w_amarillo, fmin(od_medio, fmin(nh4_elevado, fmin(no3_bajo, turb_baja)))); 
  w_amarillo = fmax(w_amarillo, fmin(dt_anomalia, fmin(od_medio, fmin(nh4_bajo, fmax(turb_baja, turb_cristalina))))); 

  // Defuzzificación
  float numerador = 0.0, denominador = 0.0;
  for (int x = 0; x <= 100; x += 2) { 
    float out_verde    = fmin(w_verde,    trapmf(x, 0, 0, 20, 45));
    float out_amarillo = fmin(w_amarillo, trapmf(x, 35, 50, 50, 65));
    float out_rojo     = fmin(w_rojo,     trapmf(x, 55, 80, 100, 100));

    float max_val = fmax(out_verde, fmax(out_amarillo, out_rojo));
    numerador += x * max_val;
    denominador += max_val;
  }
  return (denominador > 0) ? (numerador / denominador) : 0.0;
}

// =================================================================
// SECCIÓN 3: MODELOS COMPLEMENTARIOS
// =================================================================
float calcularIndiceMatematico(float dt, float od, float nh4, float no3, float turb) {
  const float LIM_OD = 10.0, LIM_NH4 = 25.0, LIM_NO3 = 15.0, LIM_TURB = 5.0, LIM_DT = 6.0;
  const float P_NH4 = 0.35, P_OD = 0.25, P_NO3 = 0.20, P_TURB = 0.10, P_DT = 0.10;

  float n_od   = 1.0 - constrain(od / LIM_OD, 0.0, 1.0);
  float n_nh4  = constrain(nh4 / LIM_NH4, 0.0, 1.0);
  float n_no3  = constrain(no3 / LIM_NO3, 0.0, 1.0);
  float n_turb = constrain(turb / LIM_TURB, 0.0, 1.0);
  float n_dt   = constrain(dt / LIM_DT, 0.0, 1.0);

  return ((n_od * P_OD) + (n_nh4 * P_NH4) + (n_no3 * P_NO3) + (n_turb * P_TURB) + (n_dt * P_DT)) * 100.0;
}

float calcularIndiceTRIX(float dt, float od, float nh, float no) {
  float nutrientes = (nh + no) + 0.01;
  float d_temp = dt + 0.01;
  float od_inv = (10.0 - od) + 0.01;
  float k = 1.5;  
  float resultado = (log10(nutrientes * od_inv * d_temp) + k) / 0.05;
  return constrain(resultado, 0.0, 100.0);
}

float calcularProbabilidadReal(float dt, float od, float nh, float no, float turb) {
  float p = 0;
  if (od < 3.0) p += 30;  
  if (nh > 4.0) p += 20;  
  if (dt > 1.2) p += 10;  
  if (no > 1.5) p += 10;  
  if (turb > 2.0) p += 30; 
  return (p > 100) ? 100 : p;
}
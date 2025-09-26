#pragma once
// Activa con: #define PCH_SIMULACION 1 antes de incluir cabeceras

#if defined(PCH_SIMULACION)
  inline float sim_tiempo_s() { return millis() / 1000.0f; }

  inline float sim_nivel(float piso, float techo, float fase = 0.0f) {
    float a = (techo - piso) * 0.4f;
    float m = (techo + piso) * 0.5f;
    return m + a * sinf(sim_tiempo_s() * 0.3f + fase);
  }

  inline float sim_caudal(float base, float amp, float fase = 0.0f) {
    return base + amp * (0.5f + 0.5f * sinf(sim_tiempo_s() * 0.2f + fase));
  }
#endif

#pragma once

// 🔐 Credenciales Firebase
extern const char key[];       // Firebase Web API Key
extern const char email[];     // Usuario autenticado
extern const char password[];  // Contraseña del usuario

// 🚫 RTDB ya no se usa → pero la mantenemos por compatibilidad con tu código actual
extern const char url[];       // NO usado por Firestore

// 🌎 Firestore requiere el Project ID de Firebase
extern const char projectId[]; // Muy importante -> lo agregamos aquí
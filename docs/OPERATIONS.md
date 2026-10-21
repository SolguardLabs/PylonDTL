# Operación y conciliación

El binario publica JSON determinista para catálogos, sesiones, órdenes,
recibos, exposición, ledger y cuotas. El servicio anfitrión debe aportar
autenticación, secuenciación, persistencia y conectores de settlement.

## Ciclo recomendado

1. Fijar versión de binario, catálogo y epoch para el proceso.
2. Inspeccionar operador, asset, canal y política antes de abrir una sesión.
3. Registrar orden y recibo, incluidos fee, reserva y beneficiario.
4. Comparar exposición agregada, cuota publicada, consumos de sesión y
   balance del asset en el mismo corte de reporte.
5. Cerrar sesiones inactivas y archivar script, JSON y digest.

## Métricas y alertas

| Métrica | Fuente | Control |
| --- | --- | --- |
| Exposición global | `exposure.global` | Evolución entre órdenes |
| Exposición por política | `exposure.by_policy` | Comparación con cuota |
| Órdenes rechazadas | `stats.orders_rejected` | Salud de admisión |
| Reservas | `stats.reserved_notional` | Cobertura por canal |
| Conservación | `ledger.conservation_ok` | Integridad contable |
| Cuota vigente | `surface.quota_log` | Capacidad publicada |

Un digest estable permite comparar reproducciones idénticas, pero no
representa consenso distribuido ni aprobación comercial. Ante divergencia,
detener la admisión externa, conservar la entrada original y reproducir el
escenario con el mismo binario antes de modificar políticas.

## Release

`main`, `production` y el tag anotado deben apuntar al mismo commit. La CI
verifica formato, compilación C++17 y tests de integración; el workflow de
integridad comprueba las referencias y los documentos públicos del release.

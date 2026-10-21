# Scripting

El CLI acepta scripts de texto con comandos separados por espacios. Las lineas
vacias y las que empiezan por `#` se ignoran.

## Comandos

```text
SCENARIO <name>
OPEN <session> <operator> <asset> <channel> <funding_account> <mode>
ORDER <order> <session> <operator> <asset> <channel> <source> <beneficiary> <notional> <units> <max_fee_bps> <priority>
QUOTE <operator> <asset> <channel> <notional> <fee_bps>
EPOCH <label> <risk_bps> <liquidity_bps> <settlement_bps>
CLOSE <session>
DEPOSIT <account> <asset> <amount>
NOTE <free text>
```

## Ejemplo

```text
SCENARIO steady-script
OPEN ses-script atlas USDC rtgs atlas-funding balanced
ORDER ord-script ses-script atlas USDC rtgs atlas-funding beneficiary-a 150000000 150000000 10 5
QUOTE atlas USDC rtgs 75000000 10
NOTE scripted rtgs settlement completed
```

Ejecutar:

```bash
dist/pylon-dtl script examples/steady-flow.pylon
```

Una línea inválida termina con error y número de línea. Los importes fuera del
rango entero de 64 bits se rechazan durante el parseo; no se truncan. Para
reproducir un caso, archivar el script, la versión del binario y el digest del
reporte. `QUOTE` es una vista puntual, no una reserva de capacidad; antes de
ejecutar una orden el integrador debe volver a consultar el estado que gobierna
su decisión operativa.

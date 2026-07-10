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

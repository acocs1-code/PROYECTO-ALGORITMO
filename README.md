# PROYECTO-ALGORITMO
INVENTARIO Y VENTAS DE FARMACIA 

Sistema en C++ (consola) para una casa médica y sus farmacias clientes.

## Cómo compilar

```bash
g++ -std=c++11 casa_medica.cpp -o casa_medica
```

## Usuarios de prueba

| Usuario | Clave | Tipo |
|---------|-------|------|
| admin   | 1234  | Casa médica |
| central | 1111  | Farmacia Central |

## Flujo

1. La farmacia hace un pedido (lista de productos que necesita).
2. La casa médica recibe la notificación y genera el presupuesto.
3. La farmacia acepta o rechaza el presupuesto.
4. Al aceptar se registra la venta: sale del inventario de la casa médica (primero el lote que vence antes) y entra al inventario de la farmacia.
5. La farmacia registra ventas al público; solo aparecen los productos que tiene en existencia.

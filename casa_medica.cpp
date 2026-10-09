#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
using namespace std;

// ---------------------------------------------------------------
// Limites de los arreglos
// ---------------------------------------------------------------
const int MAX_USUARIOS  = 50;
const int MAX_PRODUCTOS = 100;
const int MAX_LOTES     = 300;
const int MAX_CLIENTES  = 50;
const int MAX_ITEMS     = 20;   // productos por pedido / venta
const int MAX_PEDIDOS   = 100;
const int MAX_VENTAS    = 300;
const int MAX_INV_FARM  = 500;  // registros de inventario de todas las farmacias

// Estados de un pedido
const int PENDIENTE     = 0;  // la farmacia lo envio, la casa medica no ha respondido
const int PRESUPUESTADO = 1;  // la casa medica ya genero el presupuesto
const int ACEPTADO      = 2;  // la farmacia acepto -> se registro la venta
const int RECHAZADO     = 3;  // la farmacia rechazo el presupuesto

// Ganancia que la farmacia le pone al precio de la casa medica al vender al publico
const float MARGEN_FARMACIA = 0.30f;

// ---------------------------------------------------------------
// Estructuras
// ---------------------------------------------------------------
struct Usuario {
    string nombre;
    string clave;
    int tipo;            // 1 = casa medica, 2 = farmacia
    string nomFarmacia;  // solo se usa si tipo == 2
};

struct Producto {
    string codProducto;
    string nombre;
    float precio;        // precio de la casa medica
};

struct Lote {
    string codProducto;
    int numLote;
    int stock;
    string fechaVencimiento;   // formato AAAA-MM-DD
};

struct Cliente {               // solo existe en el area de la casa medica
    string nombreFarmacia;
    string telefono;
    string direccion;
};

struct ItemPedido {
    string codProducto;
    int cantidad;        // lo que pide la farmacia
    int cantAprobada;    // lo que la casa medica puede surtir (presupuesto)
    float precio;        // precio unitario del presupuesto
};

// El pedido lo hace la farmacia; el presupuesto es ese mismo pedido
// con las cantidades aprobadas y los precios que pone la casa medica.
struct Pedido {
    int id;
    string nomFarmacia;
    ItemPedido items[MAX_ITEMS];
    int numItems;
    int estado;
    float total;
};

struct ItemVenta {
    string codProducto;
    int cantidad;
    float precio;
};

struct Venta {
    int id;
    string vendedor;     // "CASA MEDICA" o nombre de la farmacia
    string comprador;    // nombre de la farmacia o "PUBLICO"
    ItemVenta items[MAX_ITEMS];
    int numItems;
    float total;
};

struct StockFarmacia {   // inventario propio de cada farmacia
    string nomFarmacia;
    string codProducto;
    int stock;
    float precioVenta;
};

// ---------------------------------------------------------------
// Datos globales
// ---------------------------------------------------------------
Usuario usuarios[MAX_USUARIOS];        int numUsuarios = 0;
Producto productos[MAX_PRODUCTOS];     int numProductos = 0;
Lote lotes[MAX_LOTES];                 int numLotes = 0;
Cliente clientes[MAX_CLIENTES];        int numClientes = 0;
Pedido pedidos[MAX_PEDIDOS];           int numPedidos = 0;
Venta ventas[MAX_VENTAS];              int numVentas = 0;
StockFarmacia invFarm[MAX_INV_FARM];   int numInvFarm = 0;

// ---------------------------------------------------------------
// Lectura de datos con validacion
// ---------------------------------------------------------------
// Quita espacios y el '\r' que deja Windows al final de la linea
string limpiar(string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == ' ')) s.pop_back();
    while (!s.empty() && s[0] == ' ') s.erase(0, 1);
    return s;
}

string leerTexto(string mensaje) {
    string s;
    do {
        cout << mensaje;
        if (!getline(cin, s)) exit(0);   // se cerro la entrada
        s = limpiar(s);
    } while (s.empty());
    return s;
}

int leerEntero(string mensaje, int minimo, int maximo) {
    while (true) {
        string s = leerTexto(mensaje);
        try {
            size_t pos;
            int n = stoi(s, &pos);
            if (pos == s.size() && n >= minimo && n <= maximo) return n;
        } catch (...) {}
        cout << "  Valor invalido. Debe ser un numero entre " << minimo << " y " << maximo << ".\n";
    }
}

float leerFloat(string mensaje, float minimo) {
    while (true) {
        string s = leerTexto(mensaje);
        try {
            size_t pos;
            float n = stof(s, &pos);
            if (pos == s.size() && n >= minimo) return n;
        } catch (...) {}
        cout << "  Valor invalido. Debe ser un numero mayor o igual a " << minimo << ".\n";
    }
}

bool fechaValida(string f) {
    if (f.size() != 10 || f[4] != '-' || f[7] != '-') return false;
    for (int i = 0; i < 10; i++)
        if (i != 4 && i != 7 && (f[i] < '0' || f[i] > '9')) return false;
    int mes = stoi(f.substr(5, 2));
    int dia = stoi(f.substr(8, 2));
    return mes >= 1 && mes <= 12 && dia >= 1 && dia <= 31;
}

void pausa() {
    cout << "\nPresione ENTER para continuar...";
    string s;
    getline(cin, s);
}

// ---------------------------------------------------------------
// Busquedas
// ---------------------------------------------------------------
int buscarUsuario(string nombre) {
    for (int i = 0; i < numUsuarios; i++)
        if (usuarios[i].nombre == nombre) return i;
    return -1;
}

int buscarProducto(string cod) {
    for (int i = 0; i < numProductos; i++)
        if (productos[i].codProducto == cod) return i;
    return -1;
}

int buscarCliente(string nomFarmacia) {
    for (int i = 0; i < numClientes; i++)
        if (clientes[i].nombreFarmacia == nomFarmacia) return i;
    return -1;
}

int buscarPedido(int id) {
    for (int i = 0; i < numPedidos; i++)
        if (pedidos[i].id == id) return i;
    return -1;
}

int buscarStockFarmacia(string nomFarmacia, string cod) {
    for (int i = 0; i < numInvFarm; i++)
        if (invFarm[i].nomFarmacia == nomFarmacia && invFarm[i].codProducto == cod) return i;
    return -1;
}

string nombreProducto(string cod) {
    int p = buscarProducto(cod);
    return p == -1 ? "(desconocido)" : productos[p].nombre;
}

string textoEstado(int estado) {
    switch (estado) {
        case PENDIENTE:     return "PENDIENTE";
        case PRESUPUESTADO: return "PRESUPUESTO ENVIADO";
        case ACEPTADO:      return "ACEPTADO / VENDIDO";
        case RECHAZADO:     return "RECHAZADO";
    }
    return "?";
}

// ---------------------------------------------------------------
// Inventario de la casa medica (por lotes)
// ---------------------------------------------------------------
int stockTotal(string cod) {
    int total = 0;
    for (int i = 0; i < numLotes; i++)
        if (lotes[i].codProducto == cod) total += lotes[i].stock;
    return total;
}

// Descuenta del inventario sacando primero de los lotes que vencen antes.
// Las fechas AAAA-MM-DD se pueden comparar directamente como texto.
void descontarStock(string cod, int cantidad) {
    while (cantidad > 0) {
        int elegido = -1;
        for (int i = 0; i < numLotes; i++) {
            if (lotes[i].codProducto == cod && lotes[i].stock > 0) {
                if (elegido == -1 || lotes[i].fechaVencimiento < lotes[elegido].fechaVencimiento)
                    elegido = i;
            }
        }
        if (elegido == -1) return;   // no deberia pasar: se valida antes
        int saca = min(cantidad, lotes[elegido].stock);
        lotes[elegido].stock -= saca;
        cantidad -= saca;
    }
}

// ---------------------------------------------------------------
// Inventario de las farmacias
// ---------------------------------------------------------------
void agregarStockFarmacia(string nomFarmacia, string cod, int cantidad, float precioCompra) {
    int i = buscarStockFarmacia(nomFarmacia, cod);
    if (i == -1) {
        if (numInvFarm == MAX_INV_FARM) {
            cout << "  Inventario de farmacias lleno.\n";
            return;
        }
        i = numInvFarm++;
        invFarm[i].nomFarmacia = nomFarmacia;
        invFarm[i].codProducto = cod;
        invFarm[i].stock = 0;
    }
    invFarm[i].stock += cantidad;
    invFarm[i].precioVenta = precioCompra * (1 + MARGEN_FARMACIA);
}

// ---------------------------------------------------------------
// Notificaciones
// ---------------------------------------------------------------
int contarPedidosPendientes() {
    int n = 0;
    for (int i = 0; i < numPedidos; i++)
        if (pedidos[i].estado == PENDIENTE) n++;
    return n;
}

int contarPresupuestosPorResponder(string nomFarmacia) {
    int n = 0;
    for (int i = 0; i < numPedidos; i++)
        if (pedidos[i].nomFarmacia == nomFarmacia && pedidos[i].estado == PRESUPUESTADO) n++;
    return n;
}

// ---------------------------------------------------------------
// Impresion
// ---------------------------------------------------------------
void mostrarPedido(Pedido &p) {
    cout << "\nPedido #" << p.id << "  |  Farmacia: " << p.nomFarmacia
         << "  |  Estado: " << textoEstado(p.estado) << "\n";
    cout << left << setw(8) << "Codigo" << setw(25) << "Producto" << right << setw(8) << "Pide";
    if (p.estado != PENDIENTE)
        cout << setw(10) << "Surtido" << setw(10) << "Precio" << setw(12) << "Subtotal";
    cout << "\n";
    for (int i = 0; i < p.numItems; i++) {
        ItemPedido &it = p.items[i];
        cout << left << setw(8) << it.codProducto << setw(25) << nombreProducto(it.codProducto)
             << right << setw(8) << it.cantidad;
        if (p.estado != PENDIENTE)
            cout << setw(10) << it.cantAprobada << setw(10) << it.precio
                 << setw(12) << it.cantAprobada * it.precio;
        cout << "\n";
    }
    if (p.estado != PENDIENTE)
        cout << right << setw(73) << "TOTAL: Q" << p.total << "\n";
}

void mostrarVenta(Venta &v) {
    cout << "\nVenta #" << v.id << "  |  Vendedor: " << v.vendedor << "  |  Comprador: " << v.comprador << "\n";
    for (int i = 0; i < v.numItems; i++) {
        cout << "  " << left << setw(8) << v.items[i].codProducto << setw(25) << nombreProducto(v.items[i].codProducto)
             << right << setw(6) << v.items[i].cantidad << " x Q" << setw(8) << v.items[i].precio
             << " = Q" << v.items[i].cantidad * v.items[i].precio << "\n";
    }
    cout << "  TOTAL: Q" << v.total << "\n";
}

void listarVentas(string vendedor) {
    cout << "\n===== HISTORIAL DE VENTAS =====\n";
    bool hay = false;
    for (int i = 0; i < numVentas; i++) {
        if (ventas[i].vendedor == vendedor) {
            mostrarVenta(ventas[i]);
            hay = true;
        }
    }
    if (!hay) cout << "No hay ventas registradas.\n";
}

// ===============================================================
//                       CASA MEDICA
// ===============================================================
void registrarProducto() {
    if (numProductos == MAX_PRODUCTOS) { cout << "No hay espacio para mas productos.\n"; return; }
    Producto p;
    p.codProducto = leerTexto("Codigo del producto: ");
    if (buscarProducto(p.codProducto) != -1) { cout << "Ese codigo ya existe.\n"; return; }
    p.nombre = leerTexto("Nombre: ");
    p.precio = leerFloat("Precio (Q): ", 0.01f);
    productos[numProductos++] = p;
    cout << "Producto registrado.\n";
}

void listarProductos() {
    cout << "\n" << left << setw(8) << "Codigo" << setw(25) << "Nombre"
         << right << setw(10) << "Precio" << setw(10) << "Stock" << "\n";
    for (int i = 0; i < numProductos; i++) {
        cout << left << setw(8) << productos[i].codProducto << setw(25) << productos[i].nombre
             << right << setw(10) << productos[i].precio << setw(10) << stockTotal(productos[i].codProducto) << "\n";
    }
    if (numProductos == 0) cout << "No hay productos registrados.\n";
}

void registrarLote() {
    if (numLotes == MAX_LOTES) { cout << "No hay espacio para mas lotes.\n"; return; }
    if (numProductos == 0) { cout << "Primero registre productos.\n"; return; }
    listarProductos();
    Lote l;
    l.codProducto = leerTexto("\nCodigo del producto: ");
    if (buscarProducto(l.codProducto) == -1) { cout << "Producto no existe.\n"; return; }
    l.numLote = leerEntero("Numero de lote: ", 1, 999999);
    for (int i = 0; i < numLotes; i++) {
        if (lotes[i].codProducto == l.codProducto && lotes[i].numLote == l.numLote) {
            cout << "Ese lote ya existe para este producto.\n";
            return;
        }
    }
    l.stock = leerEntero("Cantidad: ", 1, 1000000);
    do {
        l.fechaVencimiento = leerTexto("Fecha de vencimiento (AAAA-MM-DD): ");
    } while (!fechaValida(l.fechaVencimiento));
    lotes[numLotes++] = l;
    cout << "Lote registrado.\n";
}

void verInventario() {
    cout << "\n===== INVENTARIO POR LOTES =====\n";
    for (int p = 0; p < numProductos; p++) {
        string cod = productos[p].codProducto;
        cout << "\n" << cod << " - " << productos[p].nombre << "  (stock total: " << stockTotal(cod) << ")\n";
        for (int i = 0; i < numLotes; i++) {
            if (lotes[i].codProducto == cod && lotes[i].stock > 0)
                cout << "   Lote " << setw(6) << lotes[i].numLote << "  |  stock " << setw(6) << lotes[i].stock
                     << "  |  vence " << lotes[i].fechaVencimiento << "\n";
        }
    }
}

// Registrar un cliente (farmacia) tambien le crea su usuario para entrar al sistema
void registrarCliente() {
    if (numClientes == MAX_CLIENTES || numUsuarios == MAX_USUARIOS) {
        cout << "No hay espacio para mas clientes.\n";
        return;
    }
    Cliente c;
    c.nombreFarmacia = leerTexto("Nombre de la farmacia: ");
    if (buscarCliente(c.nombreFarmacia) != -1) { cout << "Esa farmacia ya esta registrada.\n"; return; }
    c.telefono = leerTexto("Telefono: ");
    c.direccion = leerTexto("Direccion: ");

    Usuario u;
    u.nombre = leerTexto("Usuario para que la farmacia ingrese al sistema: ");
    if (buscarUsuario(u.nombre) != -1) { cout << "Ese usuario ya existe.\n"; return; }
    u.clave = leerTexto("Clave: ");
    u.tipo = 2;
    u.nomFarmacia = c.nombreFarmacia;

    clientes[numClientes++] = c;
    usuarios[numUsuarios++] = u;
    cout << "Cliente y usuario registrados.\n";
}

void listarClientes() {
    cout << "\n" << left << setw(25) << "Farmacia" << setw(15) << "Telefono" << "Direccion\n";
    for (int i = 0; i < numClientes; i++)
        cout << left << setw(25) << clientes[i].nombreFarmacia << setw(15) << clientes[i].telefono
             << clientes[i].direccion << "\n";
    if (numClientes == 0) cout << "No hay clientes registrados.\n";
}

// La casa medica responde un pedido pendiente generando su presupuesto
void generarPresupuesto() {
    if (contarPedidosPendientes() == 0) { cout << "No hay pedidos pendientes.\n"; return; }

    cout << "\n===== PEDIDOS PENDIENTES =====\n";
    for (int i = 0; i < numPedidos; i++)
        if (pedidos[i].estado == PENDIENTE) mostrarPedido(pedidos[i]);

    int id = leerEntero("\nNumero de pedido a presupuestar (0 = regresar): ", 0, 1000000);
    if (id == 0) return;
    int k = buscarPedido(id);
    if (k == -1 || pedidos[k].estado != PENDIENTE) { cout << "Ese pedido no esta pendiente.\n"; return; }

    Pedido &p = pedidos[k];
    p.total = 0;
    cout << "\nPara cada producto ingrese cuanto puede surtir (maximo lo disponible).\n";
    for (int i = 0; i < p.numItems; i++) {
        ItemPedido &it = p.items[i];
        int disponible = stockTotal(it.codProducto);
        int maximo = min(it.cantidad, disponible);
        cout << "\n" << it.codProducto << " - " << nombreProducto(it.codProducto)
             << "  | pide: " << it.cantidad << "  | disponible: " << disponible << "\n";
        it.cantAprobada = leerEntero("  Cantidad a surtir (0 - " + to_string(maximo) + "): ", 0, maximo);
        it.precio = productos[buscarProducto(it.codProducto)].precio;
        p.total += it.cantAprobada * it.precio;
    }
    p.estado = PRESUPUESTADO;
    cout << "\nPresupuesto generado y enviado a la farmacia:";
    mostrarPedido(p);
}

void verTodosLosPedidos() {
    cout << "\n===== PEDIDOS Y PRESUPUESTOS =====\n";
    for (int i = 0; i < numPedidos; i++) mostrarPedido(pedidos[i]);
    if (numPedidos == 0) cout << "No hay pedidos.\n";
}

void menuCasaMedica(Usuario &u) {
    int op;
    do {
        cout << "\n========== CASA MEDICA (" << u.nombre << ") ==========\n";
        int pendientes = contarPedidosPendientes();
        if (pendientes > 0)
            cout << "  *** NOTIFICACION: tiene " << pendientes << " pedido(s) nuevo(s) de farmacias ***\n";
        cout << " 1. Registrar producto\n"
             << " 2. Ver productos\n"
             << " 3. Registrar lote (entrada de inventario)\n"
             << " 4. Ver inventario por lotes\n"
             << " 5. Registrar cliente (farmacia)\n"
             << " 6. Ver clientes\n"
             << " 7. Atender pedidos -> generar presupuesto\n"
             << " 8. Ver todos los pedidos / presupuestos\n"
             << " 9. Historial de ventas\n"
             << " 0. Cerrar sesion\n";
        op = leerEntero("Opcion: ", 0, 9);
        switch (op) {
            case 1: registrarProducto(); break;
            case 2: listarProductos(); break;
            case 3: registrarLote(); break;
            case 4: verInventario(); break;
            case 5: registrarCliente(); break;
            case 6: listarClientes(); break;
            case 7: generarPresupuesto(); break;
            case 8: verTodosLosPedidos(); break;
            case 9: listarVentas("CASA MEDICA"); break;
        }
        if (op != 0) pausa();
    } while (op != 0);
}

// ===============================================================
//                          FARMACIA
// ===============================================================

// La farmacia arma la lista de productos que necesita y la envia
void hacerPedido(Usuario &u) {
    if (numPedidos == MAX_PEDIDOS) { cout << "No se pueden registrar mas pedidos.\n"; return; }
    if (numProductos == 0) { cout << "La casa medica no tiene productos en catalogo.\n"; return; }

    Pedido p;
    p.id = numPedidos + 1;
    p.nomFarmacia = u.nomFarmacia;
    p.numItems = 0;
    p.estado = PENDIENTE;
    p.total = 0;

    cout << "\n===== CATALOGO DE LA CASA MEDICA =====\n";
    cout << left << setw(8) << "Codigo" << "Nombre\n";
    for (int i = 0; i < numProductos; i++)
        cout << left << setw(8) << productos[i].codProducto << productos[i].nombre << "\n";

    while (p.numItems < MAX_ITEMS) {
        string cod = leerTexto("\nCodigo del producto (0 = terminar): ");
        if (cod == "0") break;
        if (buscarProducto(cod) == -1) { cout << "Producto no existe.\n"; continue; }

        int cant = leerEntero("Cantidad que necesita: ", 1, 100000);

        // si ya estaba en la lista, se suma
        bool repetido = false;
        for (int i = 0; i < p.numItems; i++) {
            if (p.items[i].codProducto == cod) {
                p.items[i].cantidad += cant;
                repetido = true;
            }
        }
        if (!repetido) {
            p.items[p.numItems].codProducto = cod;
            p.items[p.numItems].cantidad = cant;
            p.items[p.numItems].cantAprobada = 0;
            p.items[p.numItems].precio = 0;
            p.numItems++;
        }
        cout << "Agregado.\n";
    }

    if (p.numItems == 0) { cout << "Pedido cancelado (sin productos).\n"; return; }
    pedidos[numPedidos++] = p;
    cout << "\nPedido #" << p.id << " enviado. La casa medica recibira la notificacion.\n";
}

// Cuando la farmacia acepta el presupuesto se registra la venta:
// sale del inventario de la casa medica y entra al de la farmacia.
void aceptarPresupuesto(Pedido &p) {
    for (int i = 0; i < p.numItems; i++) {
        if (p.items[i].cantAprobada > stockTotal(p.items[i].codProducto)) {
            cout << "Ya no hay suficiente existencia de " << nombreProducto(p.items[i].codProducto)
                 << ". Comuniquese con la casa medica.\n";
            return;
        }
    }
    if (numVentas == MAX_VENTAS) { cout << "No se pueden registrar mas ventas.\n"; return; }

    Venta v;
    v.id = numVentas + 1;
    v.vendedor = "CASA MEDICA";
    v.comprador = p.nomFarmacia;
    v.numItems = 0;
    v.total = 0;
    for (int i = 0; i < p.numItems; i++) {
        ItemPedido &it = p.items[i];
        if (it.cantAprobada == 0) continue;
        descontarStock(it.codProducto, it.cantAprobada);
        agregarStockFarmacia(p.nomFarmacia, it.codProducto, it.cantAprobada, it.precio);
        v.items[v.numItems].codProducto = it.codProducto;
        v.items[v.numItems].cantidad = it.cantAprobada;
        v.items[v.numItems].precio = it.precio;
        v.numItems++;
        v.total += it.cantAprobada * it.precio;
    }
    ventas[numVentas++] = v;
    p.estado = ACEPTADO;
    cout << "Presupuesto aceptado. Venta #" << v.id << " registrada y productos agregados a su inventario.\n";
}

void verMisPedidos(Usuario &u) {
    bool hay = false;
    for (int i = 0; i < numPedidos; i++) {
        if (pedidos[i].nomFarmacia == u.nomFarmacia) {
            mostrarPedido(pedidos[i]);
            hay = true;
        }
    }
    if (!hay) { cout << "No ha hecho pedidos.\n"; return; }
    if (contarPresupuestosPorResponder(u.nomFarmacia) == 0) return;

    int id = leerEntero("\nNumero de pedido para responder su presupuesto (0 = regresar): ", 0, 1000000);
    if (id == 0) return;
    int k = buscarPedido(id);
    if (k == -1 || pedidos[k].nomFarmacia != u.nomFarmacia || pedidos[k].estado != PRESUPUESTADO) {
        cout << "Ese pedido no tiene un presupuesto por responder.\n";
        return;
    }
    if (pedidos[k].total == 0) {
        cout << "La casa medica no pudo surtir ningun producto de este pedido. Se marca como rechazado.\n";
        pedidos[k].estado = RECHAZADO;
        return;
    }
    int r = leerEntero("1. Aceptar   2. Rechazar : ", 1, 2);
    if (r == 1) aceptarPresupuesto(pedidos[k]);
    else {
        pedidos[k].estado = RECHAZADO;
        cout << "Presupuesto rechazado.\n";
    }
}

void verInventarioFarmacia(Usuario &u) {
    cout << "\n===== INVENTARIO DE " << u.nomFarmacia << " =====\n";
    cout << left << setw(8) << "Codigo" << setw(25) << "Producto"
         << right << setw(8) << "Stock" << setw(12) << "Precio" << "\n";
    bool hay = false;
    for (int i = 0; i < numInvFarm; i++) {
        if (invFarm[i].nomFarmacia == u.nomFarmacia && invFarm[i].stock > 0) {
            cout << left << setw(8) << invFarm[i].codProducto << setw(25) << nombreProducto(invFarm[i].codProducto)
                 << right << setw(8) << invFarm[i].stock << setw(12) << invFarm[i].precioVenta << "\n";
            hay = true;
        }
    }
    if (!hay) cout << "Sin productos en existencia.\n";
}

// Venta al publico: solo aparecen los productos que la farmacia tiene en existencia
void ventaAlPublico(Usuario &u) {
    if (numVentas == MAX_VENTAS) { cout << "No se pueden registrar mas ventas.\n"; return; }

    Venta v;
    v.id = numVentas + 1;
    v.vendedor = u.nomFarmacia;
    v.comprador = "PUBLICO";
    v.numItems = 0;
    v.total = 0;

    while (v.numItems < MAX_ITEMS) {
        verInventarioFarmacia(u);
        string cod = leerTexto("\nCodigo del producto (0 = terminar venta): ");
        if (cod == "0") break;

        int k = buscarStockFarmacia(u.nomFarmacia, cod);
        if (k == -1 || invFarm[k].stock == 0) { cout << "Ese producto no esta en su inventario.\n"; continue; }

        // descontar lo que ya va en esta misma venta
        int yaEnVenta = 0, pos = -1;
        for (int i = 0; i < v.numItems; i++)
            if (v.items[i].codProducto == cod) { yaEnVenta = v.items[i].cantidad; pos = i; }
        int disponible = invFarm[k].stock - yaEnVenta;
        if (disponible == 0) { cout << "Ya agrego todo el stock de ese producto.\n"; continue; }

        int cant = leerEntero("Cantidad (1 - " + to_string(disponible) + "): ", 1, disponible);
        if (pos == -1) {
            pos = v.numItems++;
            v.items[pos].codProducto = cod;
            v.items[pos].cantidad = 0;
            v.items[pos].precio = invFarm[k].precioVenta;
        }
        v.items[pos].cantidad += cant;
        v.total += cant * invFarm[k].precioVenta;
        cout << "Agregado. Total parcial: Q" << v.total << "\n";
    }

    if (v.numItems == 0) { cout << "Venta cancelada.\n"; return; }

    mostrarVenta(v);
    if (leerEntero("1. Confirmar venta   2. Cancelar : ", 1, 2) == 2) {
        cout << "Venta cancelada.\n";
        return;
    }
    for (int i = 0; i < v.numItems; i++) {
        int k = buscarStockFarmacia(u.nomFarmacia, v.items[i].codProducto);
        invFarm[k].stock -= v.items[i].cantidad;
    }
    ventas[numVentas++] = v;
    cout << "Venta #" << v.id << " registrada. Inventario actualizado.\n";
}

void menuFarmacia(Usuario &u) {
    int op;
    do {
        cout << "\n========== FARMACIA " << u.nomFarmacia << " (" << u.nombre << ") ==========\n";
        int porResponder = contarPresupuestosPorResponder(u.nomFarmacia);
        if (porResponder > 0)
            cout << "  *** NOTIFICACION: la casa medica respondio " << porResponder << " pedido(s) con presupuesto ***\n";
        cout << " 1. Hacer pedido a la casa medica\n"
             << " 2. Ver mis pedidos / responder presupuestos\n"
             << " 3. Registrar venta al publico\n"
             << " 4. Ver mi inventario\n"
             << " 5. Historial de mis ventas\n"
             << " 0. Cerrar sesion\n";
        op = leerEntero("Opcion: ", 0, 5);
        switch (op) {
            case 1: hacerPedido(u); break;
            case 2: verMisPedidos(u); break;
            case 3: ventaAlPublico(u); break;
            case 4: verInventarioFarmacia(u); break;
            case 5: listarVentas(u.nomFarmacia); break;
        }
        if (op != 0) pausa();
    } while (op != 0);
}

// ===============================================================
//                       LOGIN Y DATOS
// ===============================================================
int login() {
    cout << "\n========== INICIO DE SESION ==========\n";
    string nombre = leerTexto("Usuario: ");
    string clave = leerTexto("Clave: ");
    int i = buscarUsuario(nombre);
    if (i == -1 || usuarios[i].clave != clave) {
        cout << "Usuario o clave incorrectos.\n";
        return -1;
    }
    return i;
}

void cargarDatosEjemplo() {
    usuarios[numUsuarios++] = {"admin", "1234", 1, ""};

    clientes[numClientes++] = {"Farmacia Central", "5555-1111", "Zona 1"};
    usuarios[numUsuarios++] = {"central", "1111", 2, "Farmacia Central"};

    productos[numProductos++] = {"P001", "Acetaminofen 500mg", 15.50f};
    productos[numProductos++] = {"P002", "Amoxicilina 500mg", 42.00f};
    productos[numProductos++] = {"P003", "Ibuprofeno 400mg", 22.75f};

    lotes[numLotes++] = {"P001", 101, 200, "2027-03-15"};
    lotes[numLotes++] = {"P001", 102, 150, "2026-12-01"};
    lotes[numLotes++] = {"P002", 201, 80,  "2027-06-30"};
    lotes[numLotes++] = {"P003", 301, 120, "2027-01-20"};
}

int main() {
    cout << fixed << setprecision(2);
    cargarDatosEjemplo();

    int op;
    do {
        cout << "\n===== SISTEMA CASA MEDICA / FARMACIAS =====\n"
             << " 1. Iniciar sesion\n"
             << " 0. Salir\n";
        op = leerEntero("Opcion: ", 0, 1);
        if (op == 1) {
            int i = login();
            if (i != -1) {
                if (usuarios[i].tipo == 1) menuCasaMedica(usuarios[i]);
                else menuFarmacia(usuarios[i]);
            }
        }
    } while (op != 0);

    cout << "Hasta luego.\n";
    return 0;
}

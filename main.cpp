#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <iostream>
#include <time.h>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class Estado { Inicio, Cerrar };

struct Clima {
    int estado;
    std::string descripcion;
};
std::vector<Clima> climas;
std::vector<std::vector<double>> matrizTransicion;
std::vector<int> prediccionClima;

void segundaVentana();
void interfazInicio(tgui::Gui& gui, std::function<void(int)> onValidNumber);
void abrirVentanaMatriz();
void abrirVentanaPrediccion();

std::vector<std::string> generarFechas();
std::vector<std::vector<double>> calcularMatrizTransicion();
std::vector<int> predecirClima(int dias, const std::vector<std::vector<double>>& matriz, int estadoInicial);

int main() {
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(800, 600), "Markov", sf::Style::Close);
    tgui::Gui gui(window);
    Estado estado = Estado::Inicio;

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("imagenes/background.png")) {
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(0.625f, 0.83333f);

    interfazInicio(gui, [&](int numero) {
        climas.clear();
        for (int i = 0; i < numero; i++) {
            int estado = rand() % 4;
            std::string descripcion;
            switch (estado) {
                case 0: descripcion = "Soleado"; break;
                case 1: descripcion = "P. Soleado"; break;
                case 2: descripcion = "P. Nublado"; break;
                case 3: descripcion = "Nublado"; break;
            }
            climas.push_back({estado, descripcion});
            matrizTransicion = calcularMatrizTransicion();
            prediccionClima = predecirClima(400, matrizTransicion, climas.back().estado);
        }
        window.close();
        segundaVentana();
    });

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            gui.handleEvent(event);
        }

        window.clear();
        window.draw(backgroundSprite);
        gui.draw();
        window.display();
    }

    return 0;
}

void interfazInicio(tgui::Gui& gui, std::function<void(int)> onValidNumber) {
    gui.removeAllWidgets();

    auto titulo = tgui::Label::create("Aplicación para predecir el clima");
    titulo->setTextSize(35);
    titulo->setPosition(140, 100);
    titulo->getRenderer()->setTextColor(sf::Color::White);
    gui.add(titulo);

    auto desc = tgui::Label::create("Inserte un número del 20 al 40.");
    desc->setTextSize(25);
    desc->setPosition(200, 150);
    desc->getRenderer()->setTextColor(sf::Color::White);
    gui.add(desc);

    auto input = tgui::EditBox::create();
    input->setTextSize(20);
    input->setSize(100, 40);
    input->setDefaultText("0");
    input->setPosition(360, 350);
    gui.add(input, "InputBox");

    auto boton = tgui::Button::create("Continuar");
    boton->setTextSize(22);
    boton->setSize(150, 50);
    boton->setPosition(325, 420);
    gui.add(boton);

    boton->onPress([input, onValidNumber]() {
        try {
            int numero = std::stoi(input->getText().toAnsiString());
            if (numero >= 20 && numero <= 40) {
                onValidNumber(numero);
            } else {
                auto alerta = tgui::MessageBox::create("Error", "Número inválido, debe estar entre 20 y 40.", {"OK"});
                alerta->getRenderer()->setTextColor(sf::Color::Red);
                alerta->setPosition({"(&.width - width) / 2", "(&.height - height) / 2"});
                input->getParent()->add(alerta);

                alerta->onButtonPress([alerta](const tgui::String&) {
                    alerta->getParent()->remove(alerta);
                });
            }
        } catch (...) {
            auto alerta = tgui::MessageBox::create("Error", "Entrada no válida. Ingresa un número.", {"OK"});
            alerta->getRenderer()->setTextColor(sf::Color::Red);
            alerta->setPosition({"(&.width - width) / 2", "(&.height - height) / 2"});
            input->getParent()->add(alerta);

            alerta->onButtonPress([alerta](const tgui::String&) {
                alerta->getParent()->remove(alerta);
            });
        }
    });
}

std::vector<std::vector<double>> calcularMatrizTransicion() {
    int conteo[4][4] = {0}; 
    for (size_t i = 0; i + 1 < climas.size(); i++) {
        int actual = climas[i].estado;
        int siguiente = climas[i + 1].estado;
        conteo[actual][siguiente]++;
    }

    std::vector<std::vector<double>> matriz(4, std::vector<double>(4, 0.0));
    for (int i = 0; i < 4; i++) {
        int total = 0;
        for (int j = 0; j < 4; j++) total += conteo[i][j];
        for (int j = 0; j < 4; j++) {
            if (total != 0) matriz[i][j] = static_cast<double>(conteo[i][j]) / total;
            else matriz[i][j] = 0.0;
        }
    }
    return matriz;
}
std::vector<int> predecirClima(int dias, const std::vector<std::vector<double>>& matriz, int estadoInicial) {
    std::vector<int> resultado;
    int estado = estadoInicial;
    for (int i = 0; i < dias; i++) {
        double r = (double)rand() / RAND_MAX;
        double acumulado = 0.0;
        for (int j = 0; j < 4; j++) {
            acumulado += matriz[estado][j];
            if (r <= acumulado) {
                estado = j;
                break;
            }
        }
        resultado.push_back(estado);
    }
    return resultado;
}
void segundaVentana() {
    int cantidad = climas.size();
    sf::RenderWindow nuevaVentana(sf::VideoMode(1180, 620), "Climas Aleatorios Generados", sf::Style::Close);
    tgui::Gui gui(nuevaVentana);

    auto botonMatriz = tgui::Button::create("Matriz de transicion");
    botonMatriz->setSize(200, 50);
    botonMatriz->setPosition(100, 500);
    gui.add(botonMatriz);

    auto botonPrediccion = tgui::Button::create("Generar prediccion");
    botonPrediccion->setSize(200, 50);
    botonPrediccion->setPosition(850, 500);

    botonMatriz->onPress([&]() {
        nuevaVentana.close();
        abrirVentanaMatriz(); 
    });
    gui.add(botonMatriz);
    botonPrediccion->onPress([&]() {
        nuevaVentana.close(); 
        abrirVentanaPrediccion();
    });
    gui.add(botonPrediccion);
    
    std::vector<sf::RectangleShape> rectangulos;
    std::vector<sf::Text> textosNumeros;
    std::vector<sf::Text> textosDescripcion;
    sf::Font fuente;

    if (!fuente.loadFromFile("/usr/share/fonts/TTF/Comic.TTF")) {
        return;
    }
    int columnas = 10;
    int filas = (cantidad + columnas - 1) / columnas;
    float ancho = 70;
    float alto = 60;
    float espacioX = 40;
    float espacioY = 40;
    sf::Texture soleado;
    if (!soleado.loadFromFile("imagenes/soleado.png")) {
        return;
    }
    sf::Texture parcialmente_soleado;
    if (!parcialmente_soleado.loadFromFile("imagenes/psoleado.png")) {
        return;
    }
    sf::Texture parcialmente_nublado;
    if (!parcialmente_nublado.loadFromFile("imagenes/pnublado.png")) {
        return;
    }
    sf::Texture nublado;
    if (!nublado.loadFromFile("imagenes/nublado.png")) {
        return;
    }

    for (int i = 0; i < climas.size(); i++) {
        sf::RectangleShape rect(sf::Vector2f(ancho, alto));

        sf::Texture* textura = nullptr;
        std::string descripcion = climas[i].descripcion;

        switch (climas[i].estado) {
            case 0: textura = &soleado; break;
            case 1: textura = &parcialmente_soleado; break;
            case 2: textura = &parcialmente_nublado; break;
            case 3: textura = &nublado; break;
        }
        rect.setTexture(textura);

        int col = i % columnas;
        int row = i / columnas;
        rect.setPosition(50 + col * (ancho + espacioX), 50 + row * (alto + espacioY));
        rectangulos.push_back(rect);

        sf::Text texto;
        texto.setFont(fuente);
        texto.setString(std::to_string(i + 1));
        texto.setCharacterSize(20);
        texto.setFillColor(sf::Color::White);
        texto.setPosition(rect.getPosition().x - 20, rect.getPosition().y + 5);
        textosNumeros.push_back(texto);
        
        sf::Text descripcionTexto;
        descripcionTexto.setFont(fuente);
        descripcionTexto.setString(descripcion);
        descripcionTexto.setCharacterSize(15);
        descripcionTexto.setFillColor(sf::Color::White);
        descripcionTexto.setPosition(rect.getPosition().x + 10, rect.getPosition().y + alto + 5);
        textosDescripcion.push_back(descripcionTexto);
    }
    sf::Texture fondo;
    if (!fondo.loadFromFile("imagenes/fono2.png")) {
        return;
    }
    sf::Sprite fondoSprite(fondo);
    while (nuevaVentana.isOpen()) {
        sf::Event event;
        while (nuevaVentana.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                nuevaVentana.close();

            gui.handleEvent(event);
        }

        nuevaVentana.clear();
        nuevaVentana.draw(fondoSprite);
        for (size_t i = 0; i < rectangulos.size(); i++) { 
            nuevaVentana.draw(rectangulos[i]);
            nuevaVentana.draw(textosNumeros[i]);
            nuevaVentana.draw(textosDescripcion[i]);
        }
        
        gui.draw();
        nuevaVentana.display();
    }
}


void abrirVentanaMatriz() {
    sf::RenderWindow ventana(sf::VideoMode(800, 600), "Matriz de Transición", sf::Style::Close);
    tgui::Gui gui(ventana);

    auto botonVolver = tgui::Button::create("Volver");
    botonVolver->setSize(150, 50);
    botonVolver->setPosition(20, 20);
    gui.add(botonVolver);
    
    std::vector<std::string> nombres = {"Soleado", "P. Soleado", "P. Nublado", "Nublado"};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            auto label = tgui::Label::create();
            label->setTextSize(20);
            label->setText(tgui::String(std::to_string(matrizTransicion[i][j]).substr(0,4)));
            label->setPosition(150 + j * 100, 100 + i * 50);
            gui.add(label);
        }
    }
    botonVolver->onPress([&]() {
        ventana.close();
        segundaVentana();
    });

    while (ventana.isOpen()) {
        sf::Event event;
        while (ventana.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                ventana.close();
            gui.handleEvent(event);
        }
        ventana.clear(sf::Color::White);
        gui.draw();
        ventana.display();
    }
}
std::vector<std::string> generarFechas() {
    std::vector<std::string> fechas;
    auto hoy = std::chrono::system_clock::now();
    for (int i = 0; i < 365; i++) {
        auto fecha = hoy + std::chrono::hours(24 * i);
        std::time_t t = std::chrono::system_clock::to_time_t(fecha);
        std::tm tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%d/%m/%Y");
        fechas.push_back(oss.str());
    }
    return fechas;
}

void abrirVentanaPrediccion() {
    sf::RenderWindow ventana(sf::VideoMode(1280, 800), "Predicción del Clima", sf::Style::Close);
    tgui::Gui gui(ventana);

    sf::Font fuente;
    if (!fuente.loadFromFile("/usr/share/fonts/TTF/Comic.TTF")) {
        return;
    }

    std::vector<std::string> nombres = {"Soleado", "Parcialmente\n Soleado", "Parcialmente\n Nublado", "Nublado"};
    std::vector<std::string> modos = {"Dia", "Semana", "Mes"};
    std::string modoActual = "Dia";
    std::vector<std::string> fechas = generarFechas();
    int indiceDia = 0, indiceSemana = 0, indiceMes = 0;

    sf::Texture soleado, psoleado, pnublado, nublado;
    if (!soleado.loadFromFile("imagenes/soleado.png") ||
        !psoleado.loadFromFile("imagenes/psoleado.png") ||
        !pnublado.loadFromFile("imagenes/pnublado.png") ||
        !nublado.loadFromFile("imagenes/nublado.png")) {
        return;
    }

    auto comboBox = tgui::ComboBox::create();
    comboBox->setPosition(1000, 20);
    comboBox->setSize(200, 30);
    for (const auto& modo : modos)
        comboBox->addItem(modo);
    comboBox->setSelectedItem("Dia");
    gui.add(comboBox);

    comboBox->onItemSelect([&modoActual](const tgui::String& item) {
        modoActual = item.toStdString();
    });

    auto flechaIzq = tgui::Button::create("<");
    flechaIzq->setSize(40, 40);
    flechaIzq->setPosition(600, 700);
    gui.add(flechaIzq);

    auto flechaDer = tgui::Button::create(">");
    flechaDer->setSize(40, 40);
    flechaDer->setPosition(650, 700);
    gui.add(flechaDer);

    flechaIzq->onPress([&]() {
        if (modoActual == "Dia" && indiceDia > 0) {
            --indiceDia;
        } else if (modoActual == "Semana" && indiceSemana >= 7) {
            indiceSemana -= 7;
        } else if (modoActual == "Mes") {
            if (indiceMes > 0) {
                std::tm fecha = {};
                std::istringstream ss(fechas[indiceMes]);
                ss >> std::get_time(&fecha, "%d/%m/%Y");
                int mes = fecha.tm_mon;
                int anio = fecha.tm_year + 1900;

                int diasMesAnterior;
                if (mes == 0) {
                    mes = 11;
                    --anio;
                } else {
                    --mes;
                }

                diasMesAnterior = (mes == 1) ? (((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0)) ? 29 : 28) :
                                (mes == 3 || mes == 5 || mes == 8 || mes == 10) ? 30 : 31;

                indiceMes = std::max(0, indiceMes - diasMesAnterior);
            }
        }
    });

    flechaDer->onPress([&]() {
        if (modoActual == "Dia" && indiceDia < 364) {
            ++indiceDia;
        } else if (modoActual == "Semana" && indiceSemana + 7 < 365) {
            indiceSemana += 7;
        } else if (modoActual == "Mes" && indiceMes < 365) {
            std::tm fecha = {};
            std::istringstream ss(fechas[indiceMes]);
            ss >> std::get_time(&fecha, "%d/%m/%Y");
            int mes = fecha.tm_mon;
            int anio = fecha.tm_year + 1900;

            int diasMesActual = (mes == 1) ? (((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0)) ? 29 : 28) :
                                (mes == 3 || mes == 5 || mes == 8 || mes == 10) ? 30 : 31;

            if (indiceMes + diasMesActual < (int)fechas.size()) {
                indiceMes += diasMesActual;
            }
        }
    });

    sf::Texture fondo2;
    if (!fondo2.loadFromFile("imagenes/fono2.png")) {
        return;
    }
    sf::Sprite fondito(fondo2);

    while (ventana.isOpen()) {
        sf::Event event;
        while (ventana.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                ventana.close();
            gui.handleEvent(event);
        }

        ventana.clear();
        ventana.draw(fondito);

        if (modoActual == "Dia") {
            int estado = prediccionClima[indiceDia];
            sf::Sprite imagen;
            switch (estado) {
                case 0: imagen.setTexture(soleado); break;
                case 1: imagen.setTexture(psoleado); break;
                case 2: imagen.setTexture(pnublado); break;
                case 3: imagen.setTexture(nublado); break;
            }
            imagen.setPosition(500, 220);
            imagen.setScale(0.5f, 0.5f);
            ventana.draw(imagen);

            sf::Text textoFecha;
            textoFecha.setFont(fuente);
            textoFecha.setString("Fecha: " + fechas[indiceDia]);
            textoFecha.setCharacterSize(35);
            textoFecha.setFillColor(sf::Color::White);
            textoFecha.setPosition(500, 140);
            ventana.draw(textoFecha);

            sf::Text textoPrediccion;
            textoPrediccion.setFont(fuente);
            textoPrediccion.setString(nombres[estado]);
            textoPrediccion.setCharacterSize(35);
            textoPrediccion.setFillColor(sf::Color::White);
            textoPrediccion.setPosition(550, 430);
            ventana.draw(textoPrediccion);

        } else if (modoActual == "Semana") {
            for (int i = 0; i < 7 && indiceSemana + i < 365; i++) {
                sf::RectangleShape rect(sf::Vector2f(160, 200));
                rect.setFillColor(sf::Color(111, 113, 203, 100));
                rect.setPosition(50 + i * 170, 250);
                ventana.draw(rect);

                int estado = prediccionClima[indiceSemana + i];
                sf::Sprite imagen;
                switch (estado) {
                    case 0: imagen.setTexture(soleado); break;
                    case 1: imagen.setTexture(psoleado); break;
                    case 2: imagen.setTexture(pnublado); break;
                    case 3: imagen.setTexture(nublado); break;
                }
                imagen.setPosition(rect.getPosition().x + 15, rect.getPosition().y + 20);
                imagen.setScale(0.23f, 0.23f);
                ventana.draw(imagen);

                sf::Text texto;
                texto.setFont(fuente);
                texto.setCharacterSize(18);
                texto.setString(fechas[indiceSemana + i] + "\n" + nombres[estado]);
                texto.setFillColor(sf::Color::White);
                texto.setPosition(rect.getPosition().x + 10, rect.getPosition().y + 120);
                ventana.draw(texto);
            }

        } else if (modoActual == "Mes") {
                ventana.draw(fondito);

                std::tm fechaActual = {};
                std::istringstream ss(fechas[indiceMes]);
                ss >> std::get_time(&fechaActual, "%d/%m/%Y");
                int mes = fechaActual.tm_mon + 1;
                int anio = fechaActual.tm_year + 1900;

                int diasEnMes;
                if (mes == 2) {
                    diasEnMes = ((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0)) ? 29 : 28;
                } else if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
                    diasEnMes = 30;
                } else {
                    diasEnMes = 31;
                }

                sf::Text textoMesAnio;
                textoMesAnio.setFont(fuente);
                textoMesAnio.setCharacterSize(30);
                textoMesAnio.setFillColor(sf::Color::White);
                textoMesAnio.setPosition(50, 20);
                textoMesAnio.setString("Mes: " + std::to_string(mes) + "/" + std::to_string(anio));
                ventana.draw(textoMesAnio);

                const int columnas = 6;
                const int filas = 5;
                const float ancho = 105;
                const float alto = 90;
                const float espacioX = 40;
                const float espacioY = 30;

                int inicioMes = indiceMes;

                for (int i = indiceMes; i >= 0; i--) {
                    std::tm fecha = {};
                    std::istringstream ss(fechas[i]);
                    ss >> std::get_time(&fecha, "%d/%m/%Y");
                    if ((fecha.tm_mon + 1) != mes) {
                        inicioMes = i + 1;
                        break;
                    }
                    if (i == 0) inicioMes = 0;
                }

                for (int i = 0; i < diasEnMes && (inicioMes + i) < prediccionClima.size(); i++) {
                    int col = i % columnas;
                    int row = i / columnas;

                    sf::RectangleShape rect(sf::Vector2f(ancho, alto));
                    rect.setPosition(200 + col * (ancho + espacioX), 80 + row * (alto + espacioY));

                    sf::Texture* textura = nullptr;
                    std::string descripcion;

                    int estado = prediccionClima[inicioMes + i];
                    switch (estado) {
                        case 0: textura = &soleado; descripcion = "Soleado"; break;
                        case 1: textura = &psoleado; descripcion = "P.Soleado"; break;
                        case 2: textura = &pnublado; descripcion = "P.Nublado"; break;
                        case 3: textura = &nublado; descripcion = "Nublado"; break;
                    }

                    rect.setTexture(textura);
                    ventana.draw(rect);

                    sf::Text numeroDia;
                    numeroDia.setFont(fuente);
                    numeroDia.setCharacterSize(20);
                    numeroDia.setFillColor(sf::Color::White);
                    numeroDia.setString(fechas[inicioMes + i].substr(0, 2));
                    numeroDia.setPosition(rect.getPosition().x - 20, rect.getPosition().y + 5);
                    ventana.draw(numeroDia);

                    sf::Text textoDescripcion;
                    textoDescripcion.setFont(fuente);
                    textoDescripcion.setCharacterSize(18);
                    textoDescripcion.setFillColor(sf::Color::White);
                    textoDescripcion.setString(descripcion);
                    textoDescripcion.setPosition(rect.getPosition().x + 5, rect.getPosition().y + alto + 5);
                    ventana.draw(textoDescripcion);
                }
        }
       gui.draw();
        ventana.display();
    }
}

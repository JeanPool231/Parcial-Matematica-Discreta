#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <iostream>
#include <time.h>
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
            prediccionClima = predecirClima(20, matrizTransicion, climas.back().estado);
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
    for (size_t i = 0; i + 1 < climas.size(); ++i) {
        int actual = climas[i].estado;
        int siguiente = climas[i + 1].estado;
        conteo[actual][siguiente]++;
    }

    std::vector<std::vector<double>> matriz(4, std::vector<double>(4, 0.0));
    for (int i = 0; i < 4; ++i) {
        int total = 0;
        for (int j = 0; j < 4; ++j) total += conteo[i][j];
        for (int j = 0; j < 4; ++j) {
            if (total != 0) matriz[i][j] = static_cast<double>(conteo[i][j]) / total;
            else matriz[i][j] = 0.0;
        }
    }
    return matriz;
}
std::vector<int> predecirClima(int dias, const std::vector<std::vector<double>>& matriz, int estadoInicial) {
    std::vector<int> resultado;
    int estado = estadoInicial;
    for (int i = 0; i < dias; ++i) {
        double r = (double)rand() / RAND_MAX;
        double acumulado = 0.0;
        for (int j = 0; j < 4; ++j) {
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

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
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

void abrirVentanaPrediccion() {
    sf::RenderWindow ventana(sf::VideoMode(1280, 800), "Predicción del Clima", sf::Style::Close);
    tgui::Gui gui(ventana);

    sf::Font fuente;
    if (!fuente.loadFromFile("/usr/share/fonts/TTF/Arial.TTF")) {
        return;
    }

    std::vector<std::string> nombres = {"Soleado", "P. Soleado", "P. Nublado", "Nublado"};


    while (ventana.isOpen()) {
        sf::Event event;
        while (ventana.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                ventana.close();
            gui.handleEvent(event);
        }
        ventana.clear(sf::Color::White); 
        for (size_t i = 0; i < prediccionClima.size(); i++) {
            sf::Text texto;
            texto.setFont(fuente);
            texto.setCharacterSize(20);
            texto.setFillColor(sf::Color::Black);
            texto.setPosition(100, 50 + i * 30);
            texto.setString("Dia " + std::to_string(i + 1) + ": " + nombres[prediccionClima[i]]);
            ventana.draw(texto);
        }
        gui.draw();
        ventana.display();
    }
}

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <algorithm>

const int FILAS = 25;
const int COLUMNAS = 25;
const int TAM_CELDA = 30;

enum class TipoCelda {
    VACIO,
    PARED,
    INICIO,
    FIN,
    ABIERTOS,
    CERRADOS,
    CAMINO
};

struct Nodo {
    int x, y;
    float g = 0.0f;
    float h = 0.0f;
    float f() const { return g + h; }

    bool operator>(const Nodo& otro) const {
        return f() > otro.f();
    }

    bool operator==(const Nodo& otro) const {
        return x == otro.x && y == otro.y;
    }
};

float calcularHeuristica(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

bool ejecutarAEstrella(std::vector<std::vector<TipoCelda>>& rejilla, sf::Vector2i inicio, sf::Vector2i fin) {
    std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>> listaAbierta;
    std::unordered_map<int, sf::Vector2i> padre;
    std::vector<std::vector<float>> costoG(FILAS, std::vector<float>(COLUMNAS, 1e9f));

    costoG[inicio.y][inicio.x] = 0.0f;
    listaAbierta.push({inicio.x, inicio.y, 0.0f, calcularHeuristica(inicio.x, inicio.y, fin.x, fin.y)});

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    while (!listaAbierta.empty()) {
        Nodo actual = listaAbierta.top();
        listaAbierta.pop();

        if (actual.x == fin.x && actual.y == fin.y) {
            sf::Vector2i paso = fin;
            while (!(paso == inicio)) {
                if (rejilla[paso.y][paso.x] != TipoCelda::FIN) {
                    rejilla[paso.y][paso.x] = TipoCelda::CAMINO;
                }
                int clave = paso.y * COLUMNAS + paso.x;
                paso = padre[clave];
            }
            return true;
        }

        if (rejilla[actual.y][actual.x] != TipoCelda::INICIO && rejilla[actual.y][actual.x] != TipoCelda::FIN) {
            rejilla[actual.y][actual.x] = TipoCelda::CERRADOS;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = actual.x + dx[i];
            int ny = actual.y + dy[i];

            if (nx >= 0 && nx < COLUMNAS && ny >= 0 && ny < FILAS) {
                if (rejilla[ny][nx] == TipoCelda::PARED) continue;

                float nuevoG = costoG[actual.y][actual.x] + 1.0f;

                if (nuevoG < costoG[ny][nx]) {
                    costoG[ny][nx] = nuevoG;
                    float h = calcularHeuristica(nx, ny, fin.x, fin.y);
                    
                    padre[ny * COLUMNAS + nx] = sf::Vector2i(actual.x, actual.y);
                    listaAbierta.push({nx, ny, nuevoG, h});

                    if (rejilla[ny][nx] != TipoCelda::FIN) {
                        rejilla[ny][nx] = TipoCelda::ABIERTOS;
                    }
                }
            }
        }
    }
    return false;
}

int main() {
    // SFML 3: Vector2u en el constructor de VideoMode
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(COLUMNAS * TAM_CELDA), 
                                           static_cast<unsigned int>(FILAS * TAM_CELDA)}), 
                            "Algoritmo A* en C++ (SFML 3)");

    std::vector<std::vector<TipoCelda>> rejilla(FILAS, std::vector<TipoCelda>(COLUMNAS, TipoCelda::VACIO));

    sf::Vector2i inicio(2, 2);
    sf::Vector2i fin(22, 22);

    rejilla[inicio.y][inicio.x] = TipoCelda::INICIO;
    rejilla[fin.y][fin.x] = TipoCelda::FIN;

    while (window.isOpen()) {
        // SFML 3: Bucle de eventos basado en std::optional
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    ejecutarAEstrella(rejilla, inicio, fin);
                }
                else if (keyPressed->code == sf::Keyboard::Key::R) {
                    for (int y = 0; y < FILAS; ++y) {
                        for (int x = 0; x < COLUMNAS; ++x) {
                            rejilla[y][x] = TipoCelda::VACIO;
                        }
                    }
                    rejilla[inicio.y][inicio.x] = TipoCelda::INICIO;
                    rejilla[fin.y][fin.x] = TipoCelda::FIN;
                }
            }
        }

        // SFML 3: Enums dentro de Button::
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i posMouse = sf::Mouse::getPosition(window);
            int x = posMouse.x / TAM_CELDA;
            int y = posMouse.y / TAM_CELDA;

            if (x >= 0 && x < COLUMNAS && y >= 0 && y < FILAS) {
                if (rejilla[y][x] == TipoCelda::VACIO) {
                    rejilla[y][x] = TipoCelda::PARED;
                }
            }
        }

        window.clear(sf::Color(200, 200, 200));

        sf::RectangleShape celda(sf::Vector2f(static_cast<float>(TAM_CELDA - 1), static_cast<float>(TAM_CELDA - 1)));

        for (int y = 0; y < FILAS; ++y) {
            for (int x = 0; x < COLUMNAS; ++x) {
                // SFML 3: setPosition recibe sf::Vector2f
                celda.setPosition(sf::Vector2f(static_cast<float>(x * TAM_CELDA), static_cast<float>(y * TAM_CELDA)));

                switch (rejilla[y][x]) {
                    case TipoCelda::VACIO:    celda.setFillColor(sf::Color::White); break;
                    case TipoCelda::PARED:    celda.setFillColor(sf::Color::Black); break;
                    case TipoCelda::INICIO:   celda.setFillColor(sf::Color::Green); break;
                    case TipoCelda::FIN:      celda.setFillColor(sf::Color::Red); break;
                    case TipoCelda::ABIERTOS: celda.setFillColor(sf::Color(100, 250, 100)); break;
                    case TipoCelda::CERRADOS: celda.setFillColor(sf::Color(250, 100, 100)); break;
                    case TipoCelda::CAMINO:   celda.setFillColor(sf::Color::Yellow); break;
                }

                window.draw(celda);
            }
        }

        window.display();
    }

    return 0;
}
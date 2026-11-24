#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <random>
#include <chrono>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// =========================
// Цвета
// =========================

const string RESET   = "\033[0m";
const string BOLD    = "\033[1m";
const string DIM     = "\033[2m";
const string RED     = "\033[31m";
const string GREEN   = "\033[32m";
const string YELLOW  = "\033[33m";
const string BLUE    = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN    = "\033[36m";
const string WHITE   = "\033[37m";

void enableColors() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

// =========================
// Утилиты
// =========================

string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) start++;

    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1])) end--;

    return s.substr(start, end - start);
}

string toLower(string s) {
    for (char& c : s) {
        c = (char)tolower((unsigned char)c);
    }
    return s;
}

bool isInteger(double x) {
    return fabs(x - round(x)) < 1e-12 && fabs(x) < 1e15;
}

long long gcdll(long long a, long long b) {
    a = llabs(a);
    b = llabs(b);

    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }

    return a;
}

double factorial(double x) {
    if (x >= 0 && isInteger(x) && x <= 170) {
        double r = 1.0;
        int n = (int)round(x);

        for (int i = 2; i <= n; ++i) {
            r *= i;
        }

        return r;
    }

    double r = tgamma(x + 1.0);

    if (isnan(r) || isinf(r)) {
        throw runtime_error("factorial не определён для этого значения");
    }

    return r;
}

string formatNumber(double x) {
    if (isnan(x)) return "NaN";
    if (isinf(x)) return x > 0 ? "Infinity" : "-Infinity";

    if (fabs(x) < 1e-12) x = 0.0;

    if (isInteger(x) && fabs(x) < 1e15) {
        ostringstream oss;
        oss << fixed << setprecision(0) << x;
        return oss.str();
    }

    ostringstream oss;
    oss << setprecision(12) << x;
    return oss.str();
}

// =========================
// Токены
// =========================

struct Token {
    enum Type {
        Number,
        Op,
        LParen,
        RParen,
        Ident,
        End
    };

    Type type;
    string text;
    double value;
};

vector<Token> tokenize(const string& s) {
    vector<Token> tokens;
    size_t i = 0;

    while (i < s.size()) {
        char c = s[i];

        if (isspace((unsigned char)c)) {
            i++;
            continue;
        }

        // Число, включая научную запись: 1e3, 1.5e-2
        if (isdigit((unsigned char)c) ||
            (c == '.' && i + 1 < s.size() && isdigit((unsigned char)s[i + 1]))) {

            size_t start = i;

            while (i < s.size() && (isdigit((unsigned char)s[i]) || s[i] == '.')) {
                i++;
            }

            // Научная запись
            if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
                size_t save = i;
                i++;

                if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
                    i++;
                }

                if (i < s.size() && isdigit((unsigned char)s[i])) {
                    while (i < s.size() && isdigit((unsigned char)s[i])) {
                        i++;
                    }
                } else {
                    i = save;
                }
            }

            string num = s.substr(start, i - start);

            if (count(num.begin(), num.end(), '.') > 1) {
                throw runtime_error("Неверное число: " + num);
            }

            try {
                double v = stod(num);
                tokens.push_back({Token::Number, num, v});
            } catch (...) {
                throw runtime_error("Неверное число: " + num);
            }

            continue;
        }

        // Идентификатор: pi, e, sin, ans, m и т.д.
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = i;

            while (i < s.size() && (isalnum((unsigned char)s[i]) || s[i] == '_')) {
                i++;
            }

            string id = s.substr(start, i - start);
            string lower = toLower(id);

            tokens.push_back({Token::Ident, lower, 0});
            continue;
        }

        // Операторы
        if (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '%' || c == '^' || c == '!' || c == ',') {
            tokens.push_back({Token::Op, string(1, c), 0});
            i++;
            continue;
        }

        if (c == '(') {
            tokens.push_back({Token::LParen, "(", 0});
            i++;
            continue;
        }

        if (c == ')') {
            tokens.push_back({Token::RParen, ")", 0});
            i++;
            continue;
        }

        throw runtime_error(string("Неизвестный символ: ") + c);
    }

    tokens.push_back({Token::End, "", 0});
    return tokens;
}

// =========================
// Функции
// =========================

void requireArgs(const string& name, const vector<double>& args, size_t n) {
    if (args.size() != n) {
        throw runtime_error("Функция " + name + " требует " + to_string(n) + " аргумент(ов)");
    }
}

double applyFunc(const string& name, const vector<double>& args) {
    static mt19937_64 rng(
        (unsigned long long)chrono::steady_clock::now().time_since_epoch().count()
    );

    double r = 0.0;

    // Тригонометрия
    if (name == "sin") {
        requireArgs(name, args, 1);
        r = sin(args[0]);
    } else if (name == "cos") {
        requireArgs(name, args, 1);
        r = cos(args[0]);
    } else if (name == "tan") {
        requireArgs(name, args, 1);
        r = tan(args[0]);
    } else if (name == "asin" || name == "arcsin") {
        requireArgs(name, args, 1);
        r = asin(args[0]);
    } else if (name == "acos" || name == "arccos") {
        requireArgs(name, args, 1);
        r = acos(args[0]);
    } else if (name == "atan" || name == "arctan") {
        requireArgs(name, args, 1);
        r = atan(args[0]);
    } else if (name == "atan2" || name == "arctan2") {
        requireArgs(name, args, 2);
        r = atan2(args[0], args[1]);
    } else if (name == "sec") {
        requireArgs(name, args, 1);
        double c = cos(args[0]);
        if (fabs(c) < 1e-15) throw runtime_error("sec(x) не определён");
        r = 1.0 / c;
    } else if (name == "csc") {
        requireArgs(name, args, 1);
        double s = sin(args[0]);
        if (fabs(s) < 1e-15) throw runtime_error("csc(x) не определён");
        r = 1.0 / s;
    } else if (name == "cot") {
        requireArgs(name, args, 1);
        double t = tan(args[0]);
        if (fabs(t) < 1e-15) throw runtime_error("cot(x) не определён");
        r = 1.0 / t;
    }

    // Гиперболические
    else if (name == "sinh") {
        requireArgs(name, args, 1);
        r = sinh(args[0]);
    } else if (name == "cosh") {
        requireArgs(name, args, 1);
        r = cosh(args[0]);
    } else if (name == "tanh") {
        requireArgs(name, args, 1);
        r = tanh(args[0]);
    } else if (name == "asinh") {
        requireArgs(name, args, 1);
        r = asinh(args[0]);
    } else if (name == "acosh") {
        requireArgs(name, args, 1);
        if (args[0] < 1) throw runtime_error("acosh(x) требует x >= 1");
        r = acosh(args[0]);
    } else if (name == "atanh") {
        requireArgs(name, args, 1);
        if (fabs(args[0]) >= 1) throw runtime_error("atanh(x) требует |x| < 1");
        r = atanh(args[0]);
    }

    // Степени и корни
    else if (name == "sqrt") {
        requireArgs(name, args, 1);
        if (args[0] < 0) throw runtime_error("sqrt(x) требует x >= 0");
        r = sqrt(args[0]);
    } else if (name == "cbrt" || name == "root3") {
        requireArgs(name, args, 1);
        r = cbrt(args[0]);
    } else if (name == "root" || name == "nthroot") {
        requireArgs(name, args, 2);

        double n = args[0];
        double x = args[1];

        if (n == 0) throw runtime_error("root(0, x) не определён");

        if (x < 0) {
            if (isInteger(n)) {
                long long ni = (long long)round(n);
                if (ni % 2 == 0) {
                    throw runtime_error("Чётный корень из отрицательного числа не определён");
                }
                r = -pow(-x, 1.0 / n);
            } else {
                throw runtime_error("Корень из отрицательного числа требует целого показателя");
            }
        } else {
            r = pow(x, 1.0 / n);
        }
    } else if (name == "pow") {
        requireArgs(name, args, 2);
        r = pow(args[0], args[1]);
    }

    // Логарифмы и экспонента
    else if (name == "ln") {
        requireArgs(name, args, 1);
        if (args[0] <= 0) throw runtime_error("ln(x) требует x > 0");
        r = log(args[0]);
    } else if (name == "log10") {
        requireArgs(name, args, 1);
        if (args[0] <= 0) throw runtime_error("log10(x) требует x > 0");
        r = log10(args[0]);
    } else if (name == "log2") {
        requireArgs(name, args, 1);
        if (args[0] <= 0) throw runtime_error("log2(x) требует x > 0");
        r = log2(args[0]);
    } else if (name == "log") {
        if (args.size() == 1) {
            if (args[0] <= 0) throw runtime_error("log(x) требует x > 0");
            r = log10(args[0]);
        } else if (args.size() == 2) {
            double base = args[0];
            double x = args[1];

            if (base <= 0 || base == 1 || x <= 0) {
                throw runtime_error("log(base, x) требует base > 0, base != 1, x > 0");
            }

            r = log(x) / log(base);
        } else {
            throw runtime_error("log требует 1 или 2 аргумента");
        }
    } else if (name == "exp") {
        requireArgs(name, args, 1);
        r = exp(args[0]);
    }

    // Специальные функции
    else if (name == "fact" || name == "factorial") {
        requireArgs(name, args, 1);
        r = factorial(args[0]);
    } else if (name == "gamma" || name == "tgamma") {
        requireArgs(name, args, 1);
        r = tgamma(args[0]);
        if (isnan(r) || isinf(r)) throw runtime_error("gamma не определён");
    } else if (name == "lgamma") {
        requireArgs(name, args, 1);
        r = lgamma(args[0]);
        if (isnan(r) || isinf(r)) throw runtime_error("lgamma не определён");
    } else if (name == "erf") {
        requireArgs(name, args, 1);
        r = erf(args[0]);
    } else if (name == "erfc") {
        requireArgs(name, args, 1);
        r = erfc(args[0]);
    }

    // Округление и модуль
    else if (name == "abs" || name == "fabs") {
        requireArgs(name, args, 1);
        r = fabs(args[0]);
    } else if (name == "floor") {
        requireArgs(name, args, 1);
        r = floor(args[0]);
    } else if (name == "ceil") {
        requireArgs(name, args, 1);
        r = ceil(args[0]);
    } else if (name == "round") {
        requireArgs(name, args, 1);
        r = round(args[0]);
    } else if (name == "trunc") {
        requireArgs(name, args, 1);
        r = trunc(args[0]);
    } else if (name == "sign") {
        requireArgs(name, args, 1);
        r = args[0] > 0 ? 1 : (args[0] < 0 ? -1 : 0);
    }

    // Деление
    else if (name == "mod" || name == "fmod" || name == "rem" || name == "remainder") {
        requireArgs(name, args, 2);
        if (args[1] == 0) throw runtime_error("Деление на ноль");
        r = fmod(args[0], args[1]);
    } else if (name == "div" || name == "floorDiv" || name == "quotient") {
        requireArgs(name, args, 2);
        if (args[1] == 0) throw runtime_error("Деление на ноль");
        r = floor(args[0] / args[1]);
    }

    // Целочисленные функции
    else if (name == "gcd") {
        requireArgs(name, args, 2);
        if (!isInteger(args[0]) || !isInteger(args[1])) {
            throw runtime_error("gcd требует целые числа");
        }
        r = (double)gcdll((long long)args[0], (long long)args[1]);
    } else if (name == "lcm") {
        requireArgs(name, args, 2);
        if (!isInteger(args[0]) || !isInteger(args[1])) {
            throw runtime_error("lcm требует целые числа");
        }

        long long a = llabs((long long)args[0]);
        long long b = llabs((long long)args[1]);

        if (a == 0 || b == 0) {
            r = 0;
        } else {
            r = (double)(a / gcdll(a, b) * b);
        }
    } else if (name == "isprime") {
        requireArgs(name, args, 1);
        if (!isInteger(args[0])) throw runtime_error("isprime требует целое число");

        long long n = (long long)args[0];

        if (n < 2) {
            r = 0;
        } else if (n == 2) {
            r = 1;
        } else if (n % 2 == 0) {
            r = 0;
        } else {
            bool prime = true;
            for (long long i = 3; i * i <= n; i += 2) {
                if (n % i == 0) {
                    prime = false;
                    break;
                }
            }
            r = prime ? 1 : 0;
        }
    } else if (name == "fib") {
        requireArgs(name, args, 1);
        if (!isInteger(args[0])) throw runtime_error("fib требует целое число");

        long long n = (long long)args[0];
        if (n < 0) throw runtime_error("fib требует n >= 0");

        double a = 0, b = 1;
        for (long long i = 0; i < n; ++i) {
            double c = a + b;
            a = b;
            b = c;
        }

        r = a;
    } else if (name == "comb" || name == "binom" || name == "ncr") {
        requireArgs(name, args, 2);
        if (!isInteger(args[0]) || !isInteger(args[1])) {
            throw runtime_error("comb требует целые числа");
        }

        long long n = (long long)args[0];
        long long k = (long long)args[1];

        if (n < 0 || k < 0 || k > n) {
            throw runtime_error("comb(n, k) требует 0 <= k <= n");
        }

        if (k > n - k) k = n - k;

        double res = 1;
        for (long long i = 1; i <= k; ++i) {
            res = res * (n - k + i) / i;
        }

        r = res;
    } else if (name == "perm" || name == "npr") {
        requireArgs(name, args, 2);
        if (!isInteger(args[0]) || !isInteger(args[1])) {
            throw runtime_error("perm требует целые числа");
        }

        long long n = (long long)args[0];
        long long k = (long long)args[1];

        if (n < 0 || k < 0 || k > n) {
            throw runtime_error("perm(n, k) требует 0 <= k <= n");
        }

        double res = 1;
        for (long long i = 0; i < k; ++i) {
            res *= (n - i);
        }

        r = res;
    }

    // Проверки
    else if (name == "isint") {
        requireArgs(name, args, 1);
        r = isInteger(args[0]) ? 1 : 0;
    } else if (name == "iszero") {
        requireArgs(name, args, 1);
        r = fabs(args[0]) < 1e-12 ? 1 : 0;
    } else if (name == "isneg") {
        requireArgs(name, args, 1);
        r = args[0] < 0 ? 1 : 0;
    } else if (name == "ispos") {
        requireArgs(name, args, 1);
        r = args[0] > 0 ? 1 : 0;
    } else if (name == "isodd") {
        requireArgs(name, args, 1);
        if (!isInteger(args[0])) throw runtime_error("isodd требует целое число");
        r = ((long long)args[0] % 2 != 0) ? 1 : 0;
    } else if (name == "iseven") {
        requireArgs(name, args, 1);
        if (!isInteger(args[0])) throw runtime_error("iseven требует целое число");
        r = ((long long)args[0] % 2 == 0) ? 1 : 0;
    } else if (name == "isfinite") {
        requireArgs(name, args, 1);
        r = isfinite(args[0]) ? 1 : 0;
    } else if (name == "isinf") {
        requireArgs(name, args, 1);
        r = isinf(args[0]) ? 1 : 0;
    } else if (name == "isnan") {
        requireArgs(name, args, 1);
        r = isnan(args[0]) ? 1 : 0;
    }

    // Логика
    else if (name == "and") {
        requireArgs(name, args, 2);
        r = (args[0] != 0 && args[1] != 0) ? 1 : 0;
    } else if (name == "or") {
        requireArgs(name, args, 2);
        r = (args[0] != 0 || args[1] != 0) ? 1 : 0;
    } else if (name == "not") {
        requireArgs(name, args, 1);
        r = (args[0] == 0) ? 1 : 0;
    } else if (name == "xor") {
        requireArgs(name, args, 2);
        r = ((args[0] != 0) ^ (args[1] != 0)) ? 1 : 0;
    } else if (name == "eq") {
        requireArgs(name, args, 2);
        r = fabs(args[0] - args[1]) < 1e-12 ? 1 : 0;
    } else if (name == "neq") {
        requireArgs(name, args, 2);
        r = fabs(args[0] - args[1]) >= 1e-12 ? 1 : 0;
    } else if (name == "lt") {
        requireArgs(name, args, 2);
        r = args[0] < args[1] ? 1 : 0;
    } else if (name == "gt") {
        requireArgs(name, args, 2);
        r = args[0] > args[1] ? 1 : 0;
    } else if (name == "le") {
        requireArgs(name, args, 2);
        r = args[0] <= args[1] ? 1 : 0;
    } else if (name == "ge") {
        requireArgs(name, args, 2);
        r = args[0] >= args[1] ? 1 : 0;
    } else if (name == "if") {
        requireArgs(name, args, 3);
        r = args[0] != 0 ? args[1] : args[2];
    }

    // Агрегаты
    else if (name == "sum") {
        if (args.empty()) throw runtime_error("sum требует хотя бы один аргумент");
        r = 0;
        for (double v : args) r += v;
    } else if (name == "avg") {
        if (args.empty()) throw runtime_error("avg требует хотя бы один аргумент");
        r = 0;
        for (double v : args) r += v;
        r /= (double)args.size();
    } else if (name == "max") {
        if (args.empty()) throw runtime_error("max требует хотя бы один аргумент");
        r = args[0];
        for (double v : args) r = max(r, v);
    } else if (name == "min") {
        if (args.empty()) throw runtime_error("min требует хотя бы один аргумент");
        r = args[0];
        for (double v : args) r = min(r, v);
    }

    // Геометрия
    else if (name == "hypot") {
        requireArgs(name, args, 2);
        r = hypot(args[0], args[1]);
    } else if (name == "deg") {
        requireArgs(name, args, 1);
        r = args[0] * 180.0 / acos(-1.0);
    } else if (name == "rad") {
        requireArgs(name, args, 1);
        r = args[0] * acos(-1.0) / 180.0;
    }

    // Ограничение
    else if (name == "clamp") {
        requireArgs(name, args, 3);
        double x = args[0];
        double lo = args[1];
        double hi = args[2];

        if (lo > hi) swap(lo, hi);

        if (x < lo) r = lo;
        else if (x > hi) r = hi;
        else r = x;
    }

    // Случайные числа
    else if (name == "rand" || name == "random") {
        if (!args.empty()) throw runtime_error("rand не требует аргументов");
        uniform_real_distribution<double> dist(0.0, 1.0);
        r = dist(rng);
    } else if (name == "randint" || name == "randrange") {
        requireArgs(name, args, 2);
        if (!isInteger(args[0]) || !isInteger(args[1])) {
            throw runtime_error("randint требует целые числа");
        }

        long long a = (long long)args[0];
        long long b = (long long)args[1];

        if (a > b) swap(a, b);

        uniform_int_distribution<long long> dist(a, b);
        r = (double)dist(rng);
    } else if (name == "dice") {
        requireArgs(name, args, 1);
        if (!isInteger(args[0])) throw runtime_error("dice требует целое число");

        long long sides = (long long)args[0];
        if (sides < 1) throw runtime_error("dice требует sides >= 1");

        uniform_int_distribution<long long> dist(1, sides);
        r = (double)dist(rng);
    } else if (name == "coin" || name == "flip") {
        if (!args.empty()) throw runtime_error("coin не требует аргументов");
        uniform_int_distribution<int> dist(0, 1);
        r = (double)dist(rng);
    }

    // Прочее
    else if (name == "fma") {
        requireArgs(name, args, 3);
        r = fma(args[0], args[1], args[2]);
    } else if (name == "copysign") {
        requireArgs(name, args, 2);
        r = copysign(args[0], args[1]);
    }

    else {
        throw runtime_error("Неизвестная функция: " + name);
    }

    if (isnan(r)) {
        throw runtime_error("Функция вернула неопределённое значение");
    }

    return r;
}

// =========================
// Парсер выражений
// =========================

class Parser {
    vector<Token> tokens;
    size_t pos;
    double ans;
    double mem;

    Token cur() const {
        if (pos >= tokens.size()) {
            return {Token::End, "", 0};
        }
        return tokens[pos];
    }

    bool matchOp(const string& op) const {
        return pos < tokens.size() &&
               tokens[pos].type == Token::Op &&
               tokens[pos].text == op;
    }

    void consume() {
        pos++;
    }

    double parseExpression() {
        double left = parseTerm();

        while (matchOp("+") || matchOp("-")) {
            string op = cur().text;
            consume();

            double right = parseTerm();

            if (op == "+") left += right;
            else left -= right;
        }

        return left;
    }

    double parseTerm() {
        double left = parseFactor();

        while (true) {
            if (matchOp("*") || matchOp("/") || matchOp("%")) {
                string op = cur().text;
                consume();

                double right = parseFactor();

                if (op == "*") {
                    left *= right;
                } else if (op == "/") {
                    if (right == 0) throw runtime_error("Деление на ноль");
                    left /= right;
                } else {
                    if (right == 0) throw runtime_error("Деление на ноль");
                    left = fmod(left, right);
                }
            }
            // Неявное умножение: 2pi, 2(3+4), (2)(3), sin(x)cos(x)
            else if (cur().type == Token::Number ||
                     cur().type == Token::Ident ||
                     cur().type == Token::LParen) {
                double right = parseFactor();
                left *= right;
            } else {
                break;
            }
        }

        return left;
    }

    double parseFactor() {
        if (matchOp("+")) {
            consume();
            return parseFactor();
        }

        if (matchOp("-")) {
            consume();
            return -parseFactor();
        }

        return parsePower();
    }

    double parsePower() {
        double base = parsePostfix();

        if (matchOp("^")) {
            consume();
            double exponent = parseFactor();
            return pow(base, exponent);
        }

        return base;
    }

    double parsePostfix() {
        double v = parsePrimary();

        while (matchOp("!")) {
            consume();
            v = applyFunc("fact", {v});
        }

        return v;
    }

    double parsePrimary() {
        Token t = cur();

        if (t.type == Token::Number) {
            consume();
            return t.value;
        }

        if (t.type == Token::LParen) {
            consume();

            double v = parseExpression();

            if (cur().type != Token::RParen) {
                throw runtime_error("Ожидалась ')'");
            }

            consume();
            return v;
        }

        if (t.type == Token::Ident) {
            string name = t.text;

            // Константы
            if (name == "pi") {
                consume();
                return acos(-1.0);
            }

            if (name == "e") {
                consume();
                return exp(1.0);
            }

            if (name == "tau") {
                consume();
                return 2.0 * acos(-1.0);
            }

            if (name == "phi" || name == "golden") {
                consume();
                return (1.0 + sqrt(5.0)) / 2.0;
            }

            if (name == "ans") {
                consume();
                return ans;
            }

            if (name == "m" || name == "mem" || name == "memory") {
                consume();
                return mem;
            }

            if (name == "true") {
                consume();
                return 1.0;
            }

            if (name == "false") {
                consume();
                return 0.0;
            }

            // Функция: имя(аргументы)
            if (pos + 1 < tokens.size() && tokens[pos + 1].type == Token::LParen) {
                consume(); // имя
                consume(); // (

                vector<double> args;

                if (cur().type != Token::RParen) {
                    args.push_back(parseExpression());

                    while (matchOp(",")) {
                        consume();
                        args.push_back(parseExpression());
                    }
                }

                if (cur().type != Token::RParen) {
                    throw runtime_error("Ожидалась ')' после аргументов функции " + name);
                }

                consume();

                return applyFunc(name, args);
            }

            consume();
            throw runtime_error("Неизвестная переменная: " + name);
        }

        throw runtime_error("Неожиданный символ: " + t.text);
    }

public:
    Parser(const string& expr, double ans_, double mem_)
        : tokens(tokenize(expr)), pos(0), ans(ans_), mem(mem_) {}

    double parse() {
        double v = parseExpression();

        if (cur().type != Token::End) {
            throw runtime_error("Неожиданный символ: " + cur().text);
        }

        if (isnan(v)) {
            throw runtime_error("Результат не определён");
        }

        return v;
    }
};

double evaluate(const string& expr, double ans, double mem) {
    Parser p(expr, ans, mem);
    return p.parse();
}

// =========================
// UI
// =========================

void printHeader() {
    cout << CYAN << "╔════════════════════════════════════════════════════════════╗\n";
    cout << "║  Beautiful C++ Calculator — All Functions Edition          ║\n";
    cout << "╚════════════════════════════════════════════════════════════╝" << RESET << "\n";

    cout << YELLOW << "Введите выражение или 'help'. Для выхода: exit\n" << RESET;
}

void printHelp() {
    cout << CYAN << "\nПоддерживается:\n" << RESET;

    cout << "  Арифметика: + - * / % ^ !\n";
    cout << "  Скобки: ( )\n";
    cout << "  Неявное умножение: 2pi, 2(3+4), (2)(3), sin(x)cos(x)\n";
    cout << "  Научная запись: 1e3, 1.5e-2\n";

    cout << "\n  Константы: pi, e, tau, phi, golden, ans, m, mem, memory, true, false\n";

    cout << "\n  Математика:\n";
    cout << "    sin cos tan asin acos atan atan2 sec csc cot\n";
    cout << "    sinh cosh tanh asinh acosh atanh\n";
    cout << "    sqrt cbrt root pow exp ln log log10 log2\n";
    cout << "    abs floor ceil round trunc sign\n";
    cout << "    mod div gcd lcm fact fib comb perm\n";
    cout << "    gamma lgamma erf erfc hypot deg rad fma copysign\n";

    cout << "\n  Проверки:\n";
    cout << "    isint iszero isneg ispos isodd iseven isfinite isinf isnan isprime\n";

    cout << "\n  Логика:\n";
    cout << "    and or not xor eq neq lt gt le ge if\n";

    cout << "\n  Агрегаты:\n";
    cout << "    sum avg max min clamp\n";

    cout << "\n  Случайное:\n";
    cout << "    rand randint dice coin\n";

    cout << "\n  Команды:\n";
    cout << "    help, history, clear, memory, exit\n";

    cout << "\n  Память:\n";
    cout << "    m = 5 + 2\n";
    cout << "    ans = m * 2\n";
    cout << "    m = sin(pi/2)\n";
    cout << "    m = 5!\n";
    cout << "    m = comb(10, 3)\n";
    cout << "    m = randint(1, 100)\n";
}

void printHistory(const vector<pair<string, double>>& history) {
    if (history.empty()) {
        cout << YELLOW << "История пуста.\n" << RESET;
        return;
    }

    cout << CYAN << "\nИстория:\n" << RESET;

    size_t start = history.size() > 10 ? history.size() - 10 : 0;

    for (size_t i = start; i < history.size(); ++i) {
        cout << GREEN << (i + 1) << ". " << RESET
             << history[i].first
             << " = "
             << MAGENTA << formatNumber(history[i].second)
             << RESET << "\n";
    }
}

bool tryAssignment(const string& line, double& ans, double& mem) {
    size_t eq = line.find('=');
    if (eq == string::npos) return false;

    string left = trim(line.substr(0, eq));
    string right = trim(line.substr(eq + 1));

    string l = toLower(left);

    if (l == "m" || l == "mem" || l == "memory") {
        double v = evaluate(right, ans, mem);
        mem = v;

        cout << GREEN << "m = " << formatNumber(mem) << RESET << "\n";
        return true;
    }

    if (l == "ans") {
        double v = evaluate(right, ans, mem);
        ans = v;

        cout << GREEN << "ans = " << formatNumber(ans) << RESET << "\n";
        return true;
    }

    return false;
}

// =========================
// main
// =========================

int main() {
    enableColors();

    printHeader();

    double ans = 0.0;
    double mem = 0.0;

    vector<pair<string, double>> history;

    string line;

    while (true) {
        cout << "\n" << CYAN << "calc> " << RESET;

        if (!getline(cin, line)) {
            break;
        }

        line = trim(line);

        if (line.empty()) {
            continue;
        }

        string lower = toLower(line);

        if (lower == "exit" || lower == "quit" || lower == "q" || lower == "выход") {
            cout << GREEN << "Пока!\n" << RESET;
            break;
        }

        if (lower == "help" || lower == "h" || lower == "помощь") {
            printHelp();
            continue;
        }

        if (lower == "history" || lower == "hist" || lower == "история") {
            printHistory(history);
            continue;
        }

        if (lower == "clear" || lower == "очистить") {
            history.clear();
            cout << YELLOW << "История очищена.\n" << RESET;
            continue;
        }

        if (lower == "memory" || lower == "mem" || lower == "память") {
            cout << CYAN << "m = " << formatNumber(mem)
                 << ", ans = " << formatNumber(ans) << RESET << "\n";
            continue;
        }

        try {
            if (tryAssignment(line, ans, mem)) {
                continue;
            }

            double result = evaluate(line, ans, mem);

            ans = result;
            history.push_back({line, result});

            cout << GREEN << "Результат: " << formatNumber(result) << RESET << "\n";
        } catch (const exception& e) {
            cout << RED << "Ошибка: " << e.what() << RESET << "\n";
        }
    }

    return 0;
}

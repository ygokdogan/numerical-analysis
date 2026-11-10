#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#pragma region Structs

typedef enum { NUMBER, VARIABLE, OPERATOR, FUNCTION, LEFTPAR, RIGHTPAR } TokenType;

typedef struct {
    TokenType type;
    char text[50];
    double value;
} Token;

typedef struct {
    Token* postfixTokens;
    int tokenCount;
    int decimalPointCount;
} Function;

typedef struct {
    int rows;
    int cols;
    double** data;
} Matrix;

typedef struct {
    double root;
    double error;
    int iterations;
} RootResult;

typedef struct { 
    double x0;
    double root;
    double error;
    int iterations;
} NewtonResult;

typedef struct { 
    double lowerBound;
    double higherBound;
    int intervalCount;
    double result;
} IntegralResult;


typedef struct {
    double x;
    double h;

    double forward;
    double backward;
    double central;
} DerivativeResult;

typedef struct {
    int size;
    double **A;
    double *B;
    double *X;
    double tolerance;

    int maxIteration;
} LinearSystem;

typedef struct {
    int size;

    double** original;
    double** inverse;
} InverseMatrix;

typedef struct {
    int size;
    double** A;
    double** L;
    double** U;
} CholeskyData;

typedef struct {
    int pointCount;
    double *x;
    double *y;

    double **differenceTable;
} InterpolationData;

typedef struct {
    char items[500];
    int top;
} CharStack;

typedef struct {
    double items[500];
    int top;
} DoubleStack;

typedef struct {
    Token items[500];
    int top;
} TokenStack;

#pragma endregion

#pragma region Helper Functions

Matrix* allocateMatrix(int rows, int cols) {
    int i;

    Matrix* m = (Matrix*) malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->data = (double**) malloc(rows * sizeof(double*));

    for (i = 0; i < rows; i++) {
        m->data[i] = (double*) malloc(cols * sizeof(double));
    }
    return m;
}

void freeMatrix(Matrix* matrix) {
    int i;
    for(i = 0; i < matrix->rows; i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}

void initCharStack(CharStack *s) {
    s->top = -1;
}

int isCharStackEmpty(CharStack *s) {
    return s->top == -1;
}

int isCharStackFull(CharStack *s) {
    return s->top == 499;
}

void pushChar(CharStack *s, char c) {
    if (!isCharStackFull(s)) {
        s->items[++(s->top)] = c;
    }
}

char popChar(CharStack *s) {
    if (!isCharStackEmpty(s)) {
        return s->items[(s->top)--];
    }
    return '\0';
}

char peekChar(CharStack *s) {
    if (!isCharStackEmpty(s)) {
        return s->items[s->top];
    }
    return '\0';
}

void initDoubleStack(DoubleStack *s) {
    s->top = -1;
}

int isDoubleStackEmpty(DoubleStack *s) {
    return s->top == -1;
}

int isDoubleStackFull(DoubleStack *s) {
    return s->top == 499;
}

void pushDouble(DoubleStack *s, double value) {
    if (!isDoubleStackFull(s)) {
        s->items[++(s->top)] = value;
    }
}

double popDouble(DoubleStack *s) {
    if (!isDoubleStackEmpty(s)) {
        return s->items[(s->top)--];
    }
    return 0.0;
}

double peekDouble(DoubleStack *s) {
    if (!isDoubleStackEmpty(s)) {
        return s->items[s->top];
    }
    return 0.0;
}

void initTokenStack(TokenStack *s) {
    s->top = -1;
}

int isTokenStackEmpty(TokenStack *s) {
    return s->top == -1;
}

int isTokenStackFull(TokenStack *s) {
    return s->top == 499;
}

void pushToken(TokenStack *s, Token t) {
    if (!isTokenStackFull(s)) {
        s->items[++(s->top)] = t;
    }
}

Token popToken(TokenStack *s) {
    Token empty = {0, "", 0.0};
    if (!isTokenStackEmpty(s)) {
        return s->items[(s->top)--];
    }
    return empty;
}

Token peekToken(TokenStack *s) {
    Token empty = {0, "", 0.0};
    if (!isTokenStackEmpty(s)) {
        return s->items[s->top];
    }
    return empty;
}

int isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '_');
}

int precedence(char c) {
    if (c == '~') return 4;
    if (c == '^' || c == '_') return 3;
    if (c == '*' || c == '/') return 2;
    if (c == '+' || c == '-') return 1;
    return 0;
}

int isRightAssociative(char c) {
    return (c == '^');
}

int tokenize(char* infix, Token* tokens) {
    int i = 0;
    int tCount = 0;

    while (infix[i] != '\0') {
        if (infix[i] == ' ' || infix[i] == '\t' || infix[i] == '\n') {
            i++;
        }
        else if ((infix[i] >= '0' && infix[i] <= '9') || infix[i] == '.') {
            int b = 0;
            tokens[tCount].type = NUMBER;
            
            while ((infix[i] >= '0' && infix[i] <= '9') || infix[i] == '.') {
                tokens[tCount].text[b++] = infix[i++];
            }
            tokens[tCount].text[b] = '\0';
            tokens[tCount].value = atof(tokens[tCount].text);
            tCount++;
        }
        else if (isalpha(infix[i])) {
            int b = 0;
            while (isalpha(infix[i])) {
                tokens[tCount].text[b++] = infix[i++];
            }
            tokens[tCount].text[b] = '\0';

            if (strcmp(tokens[tCount].text, "pi") == 0) {
                tokens[tCount].type = NUMBER;
                tokens[tCount].value = 3.141592653589793;
                tCount++;
            }
            else if (strcmp(tokens[tCount].text, "x") == 0) {
                tokens[tCount].type = VARIABLE;
                tokens[tCount].value = 0.0;
                tCount++;
            } 
            else if (strcmp(tokens[tCount].text, "log") == 0 && infix[i] == '_') {
                i++;
                
                if ((infix[i] >= '0' && infix[i] <= '9') || infix[i] == '.') {
                    int tb = 0;
                    char tabanStr[20];
                    while ((infix[i] >= '0' && infix[i] <= '9') || infix[i] == '.') {
                        tabanStr[tb++] = infix[i++];
                    }
                    tabanStr[tb] = '\0';
                    
                    tokens[tCount].type = NUMBER;
                    strcpy(tokens[tCount].text, tabanStr);
                    tokens[tCount].value = atof(tabanStr);
                    tCount++;
                } 
                else if (infix[i] == 'x') {
                    tokens[tCount].type = VARIABLE;
                    strcpy(tokens[tCount].text, "x");
                    tokens[tCount].value = 0.0;
                    tCount++;
                    i++;
                }
                
                tokens[tCount].type = OPERATOR;
                strcpy(tokens[tCount].text, "_");
                tokens[tCount].value = 0.0;
                tCount++;
            } 
            else {
                tokens[tCount].type = FUNCTION;
                tCount++;
            }
        }
        else {
            tokens[tCount].text[0] = infix[i];
            tokens[tCount].text[1] = '\0';

            if (infix[i] == '(') tokens[tCount].type = LEFTPAR;
            else if (infix[i] == ')') tokens[tCount].type = RIGHTPAR;
            else if (isOperator(infix[i])) {
                if (infix[i] == '-') {
                    if (tCount == 0 || 
                        (tokens[tCount-1].type != NUMBER && 
                         tokens[tCount-1].type != VARIABLE && 
                         tokens[tCount-1].type != RIGHTPAR)) {
                        
                        tokens[tCount].text[0] = '~'; 
                    }
                }
                tokens[tCount].type = OPERATOR;
            }
            else tokens[tCount].type = OPERATOR;

            i++;
            tCount++;
        }
    }
    return tCount;
}

void tokensToPostfix(Token* infixTokens, int tokenCount, Token* postfixTokens, int* postfixCount) {
    TokenStack stack;
    initTokenStack(&stack);

    int k = 0;
    int i = 0;

    while (i < tokenCount) {
        Token t = infixTokens[i];

        if (t.type == NUMBER || t.type == VARIABLE) {
            postfixTokens[k++] = t;
        }
        else if (t.type == FUNCTION) {
            pushToken(&stack, t);
        }
        else if (t.type == LEFTPAR) {
            pushToken(&stack, t);
        }
        else if (t.type == RIGHTPAR) {
            while (!isTokenStackEmpty(&stack) && peekToken(&stack).type != LEFTPAR) {
                postfixTokens[k++] = popToken(&stack);
            }
            popToken(&stack);

            if (!isTokenStackEmpty(&stack) && peekToken(&stack).type == FUNCTION) {
                postfixTokens[k++] = popToken(&stack);
            }
        }
        else if (t.type == OPERATOR) {
            char op = t.text[0];
            int keepPopping = 1;

            while (!isTokenStackEmpty(&stack) && peekToken(&stack).type == OPERATOR && keepPopping == 1) {
                char top = peekToken(&stack).text[0];

                if ((precedence(top) > precedence(op)) ||
                    (precedence(top) == precedence(op) && !isRightAssociative(op))) {
                    postfixTokens[k++] = popToken(&stack);
                } else {
                    keepPopping = 0;
                }
            }
            pushToken(&stack, t);
        }
        i++;
    }

    while (!isTokenStackEmpty(&stack)) {
        postfixTokens[k++] = popToken(&stack);
    }

    *postfixCount = k;
}

int isDigitOrDot(char c) {
    return ((c >= '0' && c <= '9') || c == '.');
}

#pragma endregion

double evaluateFunction(Function* function, double x_value) {
    int postfixCount = function->tokenCount;
    Token* postfixTokens = function->postfixTokens;
    
    DoubleStack stack;
    initDoubleStack(&stack);

    int i = 0;
    while (i < postfixCount) {
        Token t = postfixTokens[i];

        if (t.type == NUMBER) {
            pushDouble(&stack, t.value);
        }
        else if (t.type == VARIABLE) {
            pushDouble(&stack, x_value);
        }
        else if (t.type == FUNCTION) {
            double operand = popDouble(&stack);
            double result = 0.0;

            if (strcmp(t.text, "sin") == 0) result = sin(operand);
            else if (strcmp(t.text, "cos") == 0) result = cos(operand);
            else if (strcmp(t.text, "tan") == 0) result = tan(operand);
            else if (strcmp(t.text, "cot") == 0) result = 1/tan(operand);
            else if (strcmp(t.text, "arcsin") == 0) result = asin(operand);
            else if (strcmp(t.text, "arccos") == 0) result = acos(operand);
            else if (strcmp(t.text, "arctan") == 0) result = atan(operand);
            else if (strcmp(t.text, "exp") == 0) result = exp(operand);
            else if (strcmp(t.text, "ln") == 0)  result = log(operand);
            else if (strcmp(t.text, "log") == 0) result = log10(operand);

            pushDouble(&stack, result);
        }
        else if (t.type == OPERATOR) {
            char op = t.text[0];
            
            if (op == '~') {
                double operand = popDouble(&stack);
                double result = 0.0 - operand;
                pushDouble(&stack, result);
            } 
            else {
                double b_operand = popDouble(&stack);
                double a_operand = popDouble(&stack);
                double result = 0.0;

                if (op == '+') result = a_operand + b_operand;
                else if (op == '-') result = a_operand - b_operand;
                else if (op == '*') result = a_operand * b_operand;
                else if (op == '/') result = a_operand / b_operand;
                else if (op == '^') result = pow(a_operand, b_operand);
                else if (op == '_') result = log(b_operand) / log(a_operand);

                pushDouble(&stack, result);
            }
        }
        i++;
    }
    return popDouble(&stack);
}

DerivativeResult numericDerivative(Function* function, double x, double h) {
    DerivativeResult result;
    result.x = x;
    result.h = h;

    double fx = evaluateFunction(function, x);
    double fxPlusH = evaluateFunction(function, x+h);
    double fxMinusH = evaluateFunction(function, x-h);

    double central = (fxPlusH - fxMinusH) / (2.0 * h);
    double forward = (fxPlusH - fx) / h;
    double backward = (fx - fxMinusH) / h;
    
    result.central = central;
    result.forward = forward;
    result.backward = backward;
    return result;
}

RootResult bisectionMethod(Function* function, double a, double b, double tolerance, int maxIterations) {
    RootResult result;
    result.root = 0.0;
    result.error = 1.0;
    result.iterations = 0;

    double fa = evaluateFunction(function, a);
    double fb = evaluateFunction(function, b);

    if (fa * fb >= 0.0) {
        printf("[BISECTION] Error: Root may not exist in the interval [%.2f, %.2f]. (f(a)*f(b) >= 0)\n", a, b);
    }

    double c = a;
    double old_c = c;
    int keepRunning = 1;

    while (result.iterations < maxIterations && keepRunning == 1) {
        result.iterations++;
        c = (a + b) / 2.0;
        double fc = evaluateFunction(function, c);

        if (result.iterations > 1) {
            result.error = fabs(c - old_c);
        } else {
            result.error = fabs(b - a);
        }

        old_c = c;

        if (fc == 0.0) {
            result.error = 0.0;
            keepRunning = 0;
        } else if (result.error < tolerance) {
            keepRunning = 0;
        } else {
            if (fa * fc < 0.0) {
                b = c;
                fb = fc;
            } else {
                a = c;
                fa = fc;
            }
        }
    }
    result.root = c;
    return result;
}

RootResult regulaFalsiMethod(Function* function, double a, double b, double tolerance, int maxIterations) {
    double c;
    double old_c;
    int keepRunning = 1;
    RootResult result;
    result.error = 10.0;
    result.root = 0.0;
    result.iterations = 0;
    
    double fa = evaluateFunction(function, a);
    double fb = evaluateFunction(function, b);

    if (fa * fb >= 0.0) {
        printf("[REGULA-FALSI] Error: Root may not exist in the interval [%.2f, %.2f]. (f(a)*f(b) >= 0)\n", a, b);
    }

    c = a;
    old_c = c;

    while (result.iterations < maxIterations && keepRunning == 1) {
        result.iterations++;

        if(fabs(fa-fb) == 0) {
            keepRunning = 0;
        }
        else {
            c = (b * fa - a * fb) / (fa - fb);
        }

        double fc = evaluateFunction(function, c);

        if (result.iterations > 1) {
            result.error = fabs(c - old_c);
        } else {
            result.error = fabs(b - a);
        }

        old_c = c;

        if (fc == 0.0) {
            result.error = 0.0;
            keepRunning = 0;
        } else if (result.error < tolerance) {
            keepRunning = 0;
        } else {
            if (fa * fc < 0.0) {
                b = c;
                fb = fc;
            } else {
                a = c;
                fa = fc;
            }
        }
    } 

    result.root = c;
    return result;
}

NewtonResult newtonRaphsonMethod(Function* function, double x0, double h, double tolerance, int maxIterations) {
    NewtonResult result;
    result.x0 = x0;
    result.error = 10.0;
    result.root = 0.0;
    result.iterations = 0;

    double x_current = x0;
    double x_next = x_current;
    double fx;
    double dfx;
    int keepRunning = 1;
    
    while (result.iterations < maxIterations && keepRunning == 1) {
        result.iterations++;

        fx = evaluateFunction(function, x_current);
        dfx = numericDerivative(function, x_current, h).central;

        if(fabs(dfx) == 0) {
            printf("[NEWTON-RAPHSON] Error: Derivative is 0. Newton-Raphson is not available.\n");
            keepRunning = 0;
        } else {
            x_next = x_current - (fx / dfx);
            result.error = fabs(x_next - x_current);

            x_current = x_next;

            if(result.error < tolerance) {
                keepRunning = 0;
            }
        }
    }

    result.root = x_current;
    return result;
}

InverseMatrix inverseMatrix(Matrix* originalMatrix) {
    InverseMatrix result;
    
    if(originalMatrix->rows != originalMatrix->cols) {
        printf("To inverse a matrix cols and rows must be equal. Thus column count is set to row count and matrix is row*row.");
    }
    result.size = originalMatrix->rows;
    
    result.original = (double**) malloc(result.size * sizeof(double*));
    result.inverse = (double**) malloc(result.size * sizeof(double*));
    
    int i, j, k;
    for (i = 0; i < result.size; i++) {
        result.original[i] = (double*) malloc(result.size * sizeof(double));
        result.inverse[i] = (double*) malloc(result.size * sizeof(double));
        
        for (j = 0; j < result.size; j++) {
            result.original[i][j] = originalMatrix->data[i][j];
            if (i == j) {
                result.inverse[i][j] = 1.0;
            } else {
                result.inverse[i][j] = 0.0;
            }
        }
    }

    k = 0;
    int isSingular = 0;

    while (k < result.size && isSingular == 0) {
        if (fabs(result.original[k][k]) == 0) {
            int swapRow = k + 1;
            int foundNewPivot = 0;
            
            while (swapRow < result.size && foundNewPivot == 0) {
                if (fabs(result.original[swapRow][k]) != 0) {
                    foundNewPivot = 1;
                    int col = 0;
                    while (col < result.size) {
                        double tempOrig = result.original[k][col];
                        result.original[k][col] = result.original[swapRow][col];
                        result.original[swapRow][col] = tempOrig;

                        double tempInv = result.inverse[k][col];
                        result.inverse[k][col] = result.inverse[swapRow][col];
                        result.inverse[swapRow][col] = tempInv;
                        col++;
                    }
                }
                if (foundNewPivot == 0) {
                    swapRow++;
                }
            }
            
            if (foundNewPivot == 0) {
                isSingular = 1;
            }
        }

        if (isSingular == 0) {
            double pivot = result.original[k][k];

            int j = 0;
            while (j < result.size) {
                result.original[k][j] /= pivot;
                result.inverse[k][j] /= pivot;
                j++;
            }

            int r = 0;
            while (r < result.size) {
                if (r != k) {
                    double factor = result.original[r][k];
                    int c = 0;
                    while (c < result.size) {
                        result.original[r][c] -= factor * result.original[k][c];
                        result.inverse[r][c] -= factor * result.inverse[k][c];
                        c++;
                    }
                }
                r++;
            }
        }
        k++;
    }

    if (isSingular == 1) {
        printf("INVERSE] Error: The determinant of the matrix is zero or a row is entirely zero. It is not invertible!\n");
    }

    return result;
}

LinearSystem solveGaussSeidel(Matrix* matrixA, double* vectorB, double tolerance, int maxIter) {
    LinearSystem system;
    system.size = matrixA->rows;
    system.tolerance = tolerance;
    system.maxIteration = maxIter;
    
    system.A = (double**) malloc(system.size * sizeof(double*));
    system.B = (double*) malloc(system.size * sizeof(double));
    system.X = (double*) malloc(system.size * sizeof(double));
    
    int i, j;
    for (i = 0; i < system.size; i++) {
        system.A[i] = (double*) malloc(system.size * sizeof(double));
        for (j = 0; j < system.size; j++) {
            system.A[i][j] = matrixA->data[i][j];
        }
        system.B[i] = vectorB[i];
        system.X[i] = 0.0;
    }

    int isDominant = 1;
    i = 0;
    while (i < system.size && isDominant == 1) {
        double diag = fabs(system.A[i][i]);
        double sum = 0.0;
        
        j = 0;
        while (j < system.size) {
            if (i != j) {
                sum += fabs(system.A[i][j]);
            }
            j++;
        }
        
        if (diag < sum) {
            isDominant = 0;
        }
        i++;
    }

    if (isDominant == 0) {
        printf("[GAUSS-SEIDEL] Warning: The matrix is not diagonally dominant!\n");
    }

    int iter = 0;
    int keepRunning = 1;
    int hasZeroDiagonal = 0;

    double* oldX = (double*) malloc(system.size * sizeof(double));

    while (iter < system.maxIteration && keepRunning == 1 && hasZeroDiagonal == 0) {
        int converged = 1;

        int k = 0;
        while (k < system.size) {
            oldX[k] = system.X[k];
            k++;
        }
        
        i = 0;
        while (i < system.size && hasZeroDiagonal == 0) {
            if (fabs(system.A[i][i]) < 1e-12) {
                hasZeroDiagonal = 1; 
            }
            
            if (hasZeroDiagonal == 0) {
                double sum = 0.0;
                
                j = 0;
                while (j < system.size) {
                    if (i != j) {
                        if (j < i) {
                            sum += system.A[i][j] * system.X[j];
                        } else {
                            sum += system.A[i][j] * oldX[j];
                        }
                    }
                    j++;
                }
                
                system.X[i] = (system.B[i] - sum) / system.A[i][i];
                
                if (fabs(system.X[i] - oldX[i]) >= system.tolerance) {
                    converged = 0;
                }
            }
            i++;
        }

        iter++;
        
        if (converged == 1 && hasZeroDiagonal == 0) {
            keepRunning = 0;
        }
    }

    free(oldX);

    if (hasZeroDiagonal == 1) {
        printf("[GAUSS-SEIDEL] Hata: Kosegende 0 eleman tespit edildi! Durduruldu.\n");
        i = 0;
        while (i < system.size) {
            system.X[i] = 0.0;
            i++;
        }
    }

    system.maxIteration = iter;
    return system;
}

CholeskyData choleskyDecomposition(Matrix* matrixA, double* vectorB) {
    CholeskyData data;
    data.size = matrixA->rows;
    
    data.A = (double**) malloc(data.size * sizeof(double*));
    data.L = (double**) malloc(data.size * sizeof(double*));
    data.U = (double**) malloc(data.size * sizeof(double*));
    
    double* vectorY = (double*) malloc(data.size * sizeof(double));
    double* vectorX = (double*) malloc(data.size * sizeof(double));
    
    int i = 0;
    while (i < data.size) {
        data.A[i] = (double*) malloc(data.size * sizeof(double));
        data.L[i] = (double*) malloc(data.size * sizeof(double));
        data.U[i] = (double*) malloc(data.size * sizeof(double));
        
        int j = 0;
        while (j < data.size) {
            data.A[i][j] = matrixA->data[i][j];
            data.L[i][j] = 0.0;
            if (i == j) {
                data.U[i][j] = 1.0;
            } else {
                data.U[i][j] = 0.0;
            }
            j++;
        }
        vectorY[i] = 0.0;
        vectorX[i] = 0.0;
        i++;
    }

    int i_idx = 0;
    while (i_idx < data.size) {
        int j_idx = 0;
        while (j_idx <= i_idx) {
            double sum = 0.0;
            int k = 0;
            while (k < j_idx) {
                sum += data.L[i_idx][k] * data.U[k][j_idx];
                k++;
            }
            data.L[i_idx][j_idx] = data.A[i_idx][j_idx] - sum;
            j_idx++;
        }

        j_idx = i_idx + 1;
        while (j_idx < data.size) {
            double sum = 0.0;
            int k = 0;
            while (k < i_idx) {
                sum += data.L[i_idx][k] * data.U[k][j_idx];
                k++;
            }
            
            if (fabs(data.L[i_idx][i_idx]) != 0) {
                data.U[i_idx][j_idx] = (data.A[i_idx][j_idx] - sum) / data.L[i_idx][i_idx];
            } else {
                data.U[i_idx][j_idx] = 0.0;
            }
            j_idx++;
        }
        i_idx++;
    }

    i = 0;
    while (i < data.size) {
        double sum = 0.0;
        int k = 0;
        while (k < i) {
            sum += data.L[i][k] * vectorY[k];
            k++;
        }
        if (fabs(data.L[i][i]) > 1e-12) {
            vectorY[i] = (vectorB[i] - sum) / data.L[i][i];
        } else {
            vectorY[i] = 0.0;
        }
        i++;
    }

    i = data.size - 1;
    while (i >= 0) {
        double sum = 0.0;
        int k = i + 1;
        while (k < data.size) {
            sum += data.U[i][k] * vectorX[k];
            k++;
        }
        vectorX[i] = vectorY[i] - sum;
        i--;
    }

    printf("\n------ BÖLÜM 3: CHOLESKY (LU) YÖNTEMİ ÇIKTILARI ------\n");
    printf("Matris L:\n");
    i = 0;
    while (i < data.size) {
        int j = 0;
        while (j < data.size) {
            printf("%8.3f ", data.L[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }

    printf("\nMatris U:\n");
    i = 0;
    while (i < data.size) {
        int j = 0;
        while (j < data.size) {
            printf("%8.3f ", data.U[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }

    printf("\nÇözüm Vektörleri:\n");
    i = 0;
    while (i < data.size) {
        printf("Y[%d] = %8.3f  |  X[%d] = %8.3f\n", i, vectorY[i], i, vectorX[i]);
        i++;
    }
    printf("-----------------------------------------------------\n");

    free(vectorY);
    free(vectorX);
    return data;
}

IntegralResult trapezMethod(Function* function, double lower, double higher, int n) {
    IntegralResult result;
    result.lowerBound = lower;
    result.higherBound = higher;
    result.intervalCount = n;
    result.result = 0.0;

    if(n <= 0) {
        printf("[TRAPEZ] Error: Interval count must be higher than zero.\n");
        return result;
    }

    double h = (higher - lower) / (double) n;
    double f0 = evaluateFunction(function, lower);
    double fn = evaluateFunction(function, higher);

    double sum = f0 + fn;

    int i = 1;
    while(i < n) {
        double xi = lower + (i * h);
        sum += 2.0 * evaluateFunction(function, xi);
        i++;
    }

    result.result = (h / 2.0) * sum;
    return result;
}

IntegralResult simpsons13Method(Function* function, double lower, double higher, int n) {
    IntegralResult result;
    result.lowerBound = lower;
    result.higherBound = higher;
    result.intervalCount = n;
    result.result = 0.0;

    if (n % 2 != 0) {
        printf("[SIMPSONS 1/3] Warning: The number of intervals (n) must be even for Simpson's rule. n was set to %d.\n", n + 1);
        n = n + 1;
        result.intervalCount = n;
    }

    if(n <= 0) {
        printf("[SIMPSONS 1/3] Error: Interval count must be higher than zero.\n");
        return result;
    }

    double h = (higher - lower) / (double) n;
    double f0 = evaluateFunction(function, lower);
    double fn = evaluateFunction(function, higher);

    double sum = f0 + fn;
    
    int i = 1;
    while (i < n) {
        double xi = lower + (i * h);
        double fxi = evaluateFunction(function, xi);
        if (i % 2 != 0) {
            sum += 4.0 * fxi;
        } else {
            sum += 2.0 * fxi;
        }
        i++;
    }

    result.result = (h / 3.0) * sum;
    return result;
}

IntegralResult simpsons38Method(Function* function, double lower, double higher, int n) {
    IntegralResult result;
    result.lowerBound = lower;
    result.higherBound = higher;
    result.intervalCount = n;
    result.result = 0.0;


    if(n <= 0) {
        printf("[SIMPSONS 3/8] Error: Interval count must be higher than zero.\n");
        return result;
    }

    double deltaX = (higher - lower) / (double)n;
    double totalArea = 0.0;

    int i = 0;
    while (i < n) {
        double current_lower = lower + i * deltaX;
        double current_higher = current_lower + deltaX;

        double h = deltaX / 3.0;

        double fa = evaluateFunction(function, current_lower);
        double fb = evaluateFunction(function, current_higher);
        
        double x1 = current_lower + h;
        double x2 = current_lower + 2.0 * h;

        double fx1 = evaluateFunction(function, x1);
        double fx2 = evaluateFunction(function, x2);

        double currentArea = deltaX * (fa + 3.0 * fx1 + 3.0 * fx2 + fb) / 8.0;
        
        totalArea += currentArea;
        i++;
    }

    result.result = totalArea;
    return result;
}

InterpolationData* createInterpolation(int pointCount, double* xVals, double* yVals) {
    int isValid = 1;
    
    if (pointCount > 2) {
        double expectedH = xVals[1] - xVals[0];
        int i = 1;
        while (i < pointCount - 1 && isValid == 1) {
            double currentH = xVals[i+1] - xVals[i];
            if (fabs(currentH - expectedH) != 0) {
                isValid = 0;
            }
            i++;
        }
    }

    if (isValid == 0) {
        printf("\n[ERROR] x values are not equally spaced! The Gregory-Newton method is not suitable for this dataset.\n");
        return NULL;
    }

    InterpolationData* data = (InterpolationData*) malloc(sizeof(InterpolationData));
    data->pointCount = pointCount;
    data->x = (double*) malloc(pointCount * sizeof(double));
    data->y = (double*) malloc(pointCount * sizeof(double));
    data->differenceTable = (double**) malloc(pointCount * sizeof(double*));
    
    int i = 0;
    while (i < pointCount) {
        data->x[i] = xVals[i];
        data->y[i] = yVals[i];
        data->differenceTable[i] = (double*) malloc(pointCount * sizeof(double));
        
        int j = 0;
        while (j < pointCount) {
            data->differenceTable[i][j] = 0.0;
            j++;
        }
        i++;
    }

    i = 0;
    while (i < pointCount) {
        data->differenceTable[i][0] = yVals[i];
        i++;
    }

    int j = 1;
    while (j < pointCount) {
        i = 0;
        while (i < pointCount - j) {
            data->differenceTable[i][j] = data->differenceTable[i+1][j-1] - data->differenceTable[i][j-1];
            i++;
        }
        j++;
    }
    return data;
}

double evaluateGregoryNewtonForward(InterpolationData* data, double targetX) {
    double h = data->x[1] - data->x[0];
    
    double s = (targetX - data->x[0]) / h;
    
    double sum = data->differenceTable[0][0];
    
    double term = 1.0;
    double factorial = 1.0;
    
    int i = 1;
    while (i < data->pointCount) {
        term = term * (s - (i - 1));
        factorial = factorial * i;
        
        sum += (term / factorial) * data->differenceTable[0][i];
        i++;
    }
    return sum;
}

void freeInterpolation(InterpolationData* data) {
    int i = 0;
    while (i < data->pointCount) {
        free(data->differenceTable[i]);
        i++;
    }
    free(data->differenceTable);
    free(data->x);
    free(data->y);
    free(data);
}

Function* readFunctionFromUser() {
    char infix[200];
    printf("  Enter the function: (ex: sin(x) + x^2 - 3)\n");
    printf("  NOTE: log_5(x) = log with base 5, ln(x) = natural log\n");
    printf("  >> f(x) = ");
    
    int ch = getchar();
    if (ch == '\n') {
        fgets(infix, sizeof(infix), stdin);
    } else {
        infix[0] = (char)ch;
        fgets(infix + 1, sizeof(infix) - 1, stdin);
    }
 
    int len = strlen(infix);
    if (len > 0 && infix[len - 1] == '\n') {
        infix[len - 1] = '\0';
    }
 
    Token* infixTokens = (Token*) malloc(300 * sizeof(Token));
    Token* postfixTokens = (Token*) malloc(300 * sizeof(Token));
    int infixCount = 0;
    int postfixCount = 0;
 
    infixCount = tokenize(infix, infixTokens);
    tokensToPostfix(infixTokens, infixCount, postfixTokens, &postfixCount);
 
    Function* func = (Function*) malloc(sizeof(Function));
    func->postfixTokens = postfixTokens;
    func->tokenCount = postfixCount;
 
    free(infixTokens);

    printf("  Enter decimal point count: "); scanf("%d", &func->decimalPointCount);
    return func;
}
 
void freeFunction(Function* func) {
    free(func->postfixTokens);
    free(func);
}
 
void menuBisection() {
    printf("\n=== 1. BISECTION METHOD ===\n");
 
    Function* func = readFunctionFromUser();
 
    double a, b, tol;
    int maxIter;
    printf("  Lower bound (a)    : "); scanf("%lf", &a);
    printf("  Upper bound (b)    : "); scanf("%lf", &b);
    printf("  Tolerance          : "); scanf("%lf", &tol);
    printf("  Max iterations     : "); scanf("%d", &maxIter);
 
    RootResult res = bisectionMethod(func, a, b, tol, maxIter);
 
    printf("\n--- RESULT ---\n");
    printf("  Root      : %.*f\n", func->decimalPointCount, res.root);
    printf("  Error     : %.*f\n", func->decimalPointCount, res.error);
    printf("  Iteration : %d\n", res.iterations);
 
    freeFunction(func);
}
 
void menuRegulaFalsi() {
    printf("\n=== 2. REGULA-FALSI METHOD ===\n");
 
    Function* func = readFunctionFromUser();
 
    double a, b, tol;
    int maxIter;
    printf("  Lower bound (a)    : "); scanf("%lf", &a);
    printf("  Upper bound (b)    : "); scanf("%lf", &b);
    printf("  Tolerance          : "); scanf("%lf", &tol);
    printf("  Max iterations     : "); scanf("%d",  &maxIter);
 
    RootResult res = regulaFalsiMethod(func, a, b, tol, maxIter);
 
    printf("\n--- RESULT ---\n");
    printf("  Root      : %.*f\n", func->decimalPointCount, res.root);
    printf("  Error     : %.*f\n", func->decimalPointCount, res.error);
    printf("  Iteration : %d\n", res.iterations);
 
    freeFunction(func);
}
 
void menuNewtonRaphson() {
    printf("\n=== 3. NEWTON-RAPHSON METHOD ===\n");
 
    Function* func = readFunctionFromUser();
 
    double x0, h, tol;
    int maxIter;
    printf("  Initial value (x0) : "); scanf("%lf", &x0);
    printf("  Derivative step (h): "); scanf("%lf", &h);
    printf("  Tolerance          : "); scanf("%lf", &tol);
    printf("  Max iterations     : "); scanf("%d",  &maxIter);
 
    NewtonResult res = newtonRaphsonMethod(func, x0, h, tol, maxIter);
 
    printf("\n--- RESULT ---\n");
    printf("  Initial value (x0) : %.*f\n", func->decimalPointCount, res.x0);
    printf("  Root         : %.*f\n", func->decimalPointCount, res.root);
    printf("  Error        : %.*f\n", func->decimalPointCount, res.error);
    printf("  Iteration    : %d\n",    res.iterations);
 
    freeFunction(func);
}
 
void menuInverseMatrix() {
    printf("\n=== 4. NxN MATRIX INVERSE (GAUSS-JORDAN) ===\n");
 
    int n;
    printf("  Matrix size (N) : "); scanf("%d", &n);
 
    Matrix* mat = allocateMatrix(n, n);
 
    printf("  Enter matrix elements row by row:\n");
    int i = 0;
    while (i < n) {
        int j = 0;
        while (j < n) {
            printf("    A[%d][%d] = ", i + 1, j + 1);
            scanf("%lf", &mat->data[i][j]);
            j++;
        }
        i++;
    }
 
    InverseMatrix res = inverseMatrix(mat);
 
    printf("\n--- INVERSE MATRIX ---\n");
    i = 0;
    while (i < res.size) {
        int j = 0;
        while (j < res.size) {
            printf("%10.5f ", res.inverse[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }
 
    i = 0;
    while (i < res.size) {
        free(res.original[i]);
        free(res.inverse[i]);
        i++;
    }
    free(res.original);
    free(res.inverse);
    freeMatrix(mat);
}
 
void menuCholesky() {
    printf("\n=== 5. CHOLESKY (LU) METHOD ===\n");
 
    int n;
    printf("  Matrix size (N) : "); scanf("%d", &n);
 
    Matrix* mat = allocateMatrix(n, n);
    double* vecB = (double*) malloc(n * sizeof(double));
 
    printf("  Enter coefficient matrix A elements:\n");
    int i = 0;
    while (i < n) {
        int j = 0;
        while (j < n) {
            printf("    A[%d][%d] = ", i + 1, j + 1);
            scanf("%lf", &mat->data[i][j]);
            j++;
        }
        i++;
    }
 
    printf("  Enter right hand side vector B elements:\n");
    i = 0;
    while (i < n) {
        printf("    B[%d] = ", i + 1);
        scanf("%lf", &vecB[i]);
        i++;
    }
 
    CholeskyData res = choleskyDecomposition(mat, vecB);
 
    i = 0;
    while (i < res.size) {
        free(res.A[i]);
        free(res.L[i]);
        free(res.U[i]);
        i++;
    }
    free(res.A);
    free(res.L);
    free(res.U);
    free(vecB);
    freeMatrix(mat);
}
 
void menuGaussSeidel() {
    printf("\n=== 6. GAUSS-SEIDEL METHOD ===\n");
 
    int n;
    printf("  Number of equations (N) : "); scanf("%d", &n);
 
    Matrix* mat = allocateMatrix(n, n);
    double* vecB = (double*) malloc(n * sizeof(double));
 
    printf("  Enter coefficient matrix A elements:\n");
    int i = 0;
    while (i < n) {
        int j = 0;
        while (j < n) {
            printf("    A[%d][%d] = ", i + 1, j + 1);
            scanf("%lf", &mat->data[i][j]);
            j++;
        }
        i++;
    }
 
    printf("  Enter right hand side vector B elements:\n");
    i = 0;
    while (i < n) {
        printf("    B[%d] = ", i + 1);
        scanf("%lf", &vecB[i]);
        i++;
    }
 
    double tol;
    int maxIter;
    printf("  Tolerance      : "); scanf("%lf", &tol);
    printf("  Max iterations : "); scanf("%d",  &maxIter);
 
    LinearSystem res = solveGaussSeidel(mat, vecB, tol, maxIter);
 
    printf("\n--- SOLUTION VECTOR X ---\n");
    printf("  Number of iterations: %d\n", res.maxIteration);
    i = 0;
    while (i < res.size) {
        printf("  X[%d] = %.10f\n", i + 1, res.X[i]);
        i++;
    }
 
    i = 0;
    while (i < res.size) {
        free(res.A[i]);
        i++;
    }
    free(res.A);
    free(res.B);
    free(res.X);
    free(vecB);
    freeMatrix(mat);
}
 
void menuNumericalDerivative() {
    printf("\n=== 7. NUMERICAL DERIVATIVE ===\n");
 
    Function* func = readFunctionFromUser();
 
    double x, h;
    printf("  Point of derivative (x) : "); scanf("%lf", &x);
    printf("  Step size (h)           : "); scanf("%lf", &h);
 
    DerivativeResult res = numericDerivative(func, x, h);
 
    printf("\n--- RESULTS ---\n");
    printf("  Central diff  : %.*f\n", func->decimalPointCount, res.central);
    printf("  Forward diff  : %.*f\n", func->decimalPointCount, res.forward);
    printf("  Backward diff : %.*f\n", func->decimalPointCount, res.backward);
 
    freeFunction(func);
}
 
void menuSimpson() {
    printf("\n=== 8. SIMPSON'S METHODS (1/3 and 3/8) ===\n");
 
    Function* func = readFunctionFromUser();
 
    double lower, upper;
    int n;
    printf("  Lower bound (a)         : "); scanf("%lf", &lower);
    printf("  Upper bound (b)         : "); scanf("%lf", &upper);
    printf("  Number of intervals (n) : "); scanf("%d",  &n);
 
    IntegralResult res13 = simpsons13Method(func, lower, upper, n);
    IntegralResult res38 = simpsons38Method(func, lower, upper, n);
 
    printf("\n--- RESULTS ---\n");
    printf("  Simpson 1/3 : %.*f  (n = %d)\n", func->decimalPointCount, res13.result, res13.intervalCount);
    printf("  Simpson 3/8 : %.*f  (n = %d)\n", func->decimalPointCount, res38.result, res38.intervalCount);
 
    freeFunction(func);
}
 
void menuTrapez() {
    printf("\n=== 9. TRAPEZOIDAL METHOD ===\n");
 
    Function* func = readFunctionFromUser();
 
    double lower, upper;
    int n;
    printf("  Lower bound (a)         : "); scanf("%lf", &lower);
    printf("  Upper bound (b)         : "); scanf("%lf", &upper);
    printf("  Number of intervals (n) : "); scanf("%d",  &n);
 
    IntegralResult res = trapezMethod(func, lower, upper, n);
 
    printf("\n--- RESULT ---\n");
    printf("  Area : %.*f  (n = %d)\n", func->decimalPointCount, res.result, res.intervalCount);
 
    freeFunction(func);
}
 
void menuGregoryNewton() {
    printf("\n=== 10. GREGORY-NEWTON INTERPOLATION ===\n");
 
    int n;
    printf("  Number of points : "); scanf("%d", &n);
 
    double* xVals = (double*) malloc(n * sizeof(double));
    double* yVals = (double*) malloc(n * sizeof(double));
 
    printf("  Enter the points:\n");
    int i = 0;
    while (i < n) {
        printf("    (x, y)[%d] = ", i + 1); scanf("%lf %lf", &xVals[i], &yVals[i]);
        i++;
    }
 
    InterpolationData* data = createInterpolation(n, xVals, yVals);
 
    if (data != NULL) {
        printf("\n--- DIFFERENCE TABLE ---\n");
        i = 0;
        while (i < data->pointCount) {
            printf("  x = %8.4f |", data->x[i]);
            int j = 0;
            while (j < data->pointCount - i) {
                printf(" %10.5f", data->differenceTable[i][j]);
                j++;
            }
            printf("\n");
            i++;
        }
 
        double targetX;
        printf("\n  Value of x to interpolate : "); scanf("%lf", &targetX);
 
        double result = evaluateGregoryNewtonForward(data, targetX);
        printf("\n--- RESULT ---\n");
        printf("  F(%.4f) = %10.5f\n", targetX, result);
 
        freeInterpolation(data);
    }
 
    free(xVals);
    free(yVals);
}
 
void printMenu() {
    printf("\n--- NUMERICAL ANALYSIS - TERM PROJECT 2026 ---\n");
    printf("1.  Bisection Method\n");
    printf("2.  Regula-Falsi Method\n");
    printf("3.  Newton-Raphson Method\n");
    printf("4.  Matrix Inverse (NxN, Gauss-Jordan)\n");
    printf("5.  Cholesky (LU) Method\n");
    printf("6.  Gauss-Seidel Method\n");
    printf("7.  Numerical Derivative (Forward/Backward/Central)\n");
    printf("8.  Simpson's Methods (1/3 and 3/8)\n");
    printf("9.  Trapezoidal Method\n");
    printf("10. Gregory-Newton Interpolation\n");
    printf("0.  Exit\n");
    printf("Your choice: ");
}
 
int main() {
    int choice = -1;
 
    while (choice != 0) {
        printMenu();
        scanf("%d", &choice);
 
        if      (choice == 1)  menuBisection();
        else if (choice == 2)  menuRegulaFalsi();
        else if (choice == 3)  menuNewtonRaphson();
        else if (choice == 4)  menuInverseMatrix();
        else if (choice == 5)  menuCholesky();
        else if (choice == 6)  menuGaussSeidel();
        else if (choice == 7)  menuNumericalDerivative();
        else if (choice == 8)  menuSimpson();
        else if (choice == 9)  menuTrapez();
        else if (choice == 10) menuGregoryNewton();
        else if (choice == 0)  printf("\nExiting...\n");
        else                   printf("\nInvalid choice. Please try again.\n");
    }
 
    return 0;
}
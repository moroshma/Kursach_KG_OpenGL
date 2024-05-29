#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;     // (Н) Эта переменная отвечает за смену текстур

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
    //дистанция камеры
    double camDist;
    //углы поворота камеры
    double fi1, fi2;


    //значния масеры по умолчанию
    CustomCamera()
    {
        camDist = 15;
        fi1 = 1;
        fi2 = 1;
    }


    //считает позицию камеры, исходя из углов поворота, вызывается движком
    void SetUpCamera()
    {
        //отвечает за поворот камеры мышкой
        lookPoint.setCoords(0, 0, 0);

        pos.setCoords(camDist * cos(fi2) * cos(fi1),
            camDist * cos(fi2) * sin(fi1),
            camDist * sin(fi2));

        if (cos(fi2) <= 0)
            normal.setCoords(0, 0, -1);
        else
            normal.setCoords(0, 0, 1);

        LookAt();
    }

    void CustomCamera::LookAt()
    {
        //функция настройки камеры
        gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
    }



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
    CustomLight()
    {
        //начальная позиция света
        pos = Vector3(1, 1, 3);
    }


    //рисует сферу и линии под источником света, вызывается движком
    void  DrawLightGhismo()
    {
        glDisable(GL_LIGHTING);


        glColor3d(0.9, 0.8, 0);
        Sphere s;
        s.pos = pos;
        s.scale = s.scale * 0.08;
        s.Show();

        if (OpenGL::isKeyPressed('G'))
        {
            glColor3d(0, 0, 0);
            //линия от источника света до окружности
            glBegin(GL_LINES);
            glVertex3d(pos.X(), pos.Y(), pos.Z());
            glVertex3d(pos.X(), pos.Y(), 0);
            glEnd();

            //рисуем окруность
            Circle c;
            c.pos.setCoords(pos.X(), pos.Y(), 0);
            c.scale = c.scale * 1.5;
            c.Show();
        }

    }

    void SetUpLight()
    {
        GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
        GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
        GLfloat spec[] = { .7, .7, .7, 0 };
        GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

        // параметры источника света
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        // характеристики излучаемого света
        // фоновое освещение (рассеянный свет)
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
        // диффузная составляющая света
        glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
        // зеркально отражаемая составляющая света
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

        glEnable(GL_LIGHT0);
    }


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
    int dx = mouseX - mX;
    int dy = mouseY - mY;
    mouseX = mX;
    mouseY = mY;

    //меняем углы камеры при нажатой левой кнопке мыши
    if (OpenGL::isKeyPressed(VK_RBUTTON))
    {
        camera.fi1 += 0.01 * dx;
        camera.fi2 += -0.01 * dy;
    }


    //двигаем свет по плоскости, в точку где мышь
    if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
    {
        LPPOINT POINT = new tagPOINT();
        GetCursorPos(POINT);
        ScreenToClient(ogl->getHwnd(), POINT);
        POINT->y = ogl->getHeight() - POINT->y;

        Ray r = camera.getLookRay(POINT->x, POINT->y);

        double z = light.pos.Z();

        double k = 0, x = 0, y = 0;
        if (r.direction.Z() == 0)
            k = 0;
        else
            k = (z - r.origin.Z()) / r.direction.Z();

        x = k * r.direction.X() + r.origin.X();
        y = k * r.direction.Y() + r.origin.Y();

        light.pos = Vector3(x, y, z);
    }

    if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
    {
        light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
    }


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

    if (delta < 0 && camera.camDist <= 1)
        return;
    if (delta > 0 && camera.camDist >= 100)
        return;

    camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
    if (key == 'L')
    {
        lightMode = !lightMode;
    }

    if (key == 'T')
    {
        textureMode = !textureMode;
    }

    if (key == 'R')
    {
        camera.fi1 = 1;
        camera.fi2 = 1;
        camera.camDist = 15;

        light.pos = Vector3(1, 1, 3);
    }

    if (key == 'F')
    {
        light.pos = camera.pos;
    }

    if (key == 'E')                                         // (Н) Смена текстуры происходит по этой кнопке
    {
        textureReplace = !textureReplace;
    }
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;
GLuint texId2;                                  // (Н) Я хз как по другому делать, поэтому я просто скопировал часть кода, ко всем переменным в конце добавил "2", и всё работает

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
    //настройка текстур

    //4 байта на хранение пикселя
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    //настройка режима наложения текстур
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //включаем текстуры
    glEnable(GL_TEXTURE_2D);


    //массив трехбайтных элементов  (R G B)                                                                                       (Н)    Код брался и копировался отсюда
    RGBTRIPLE* texarray;

    //массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
    char* texCharArray;
    int texW, texH;
    OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);                    // (Н) Чтоб не мучаться с картинками, я коприовал старую и рисовал на ней
    OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);


    //генерируем ИД для текстуры
    glGenTextures(1, &texId);
    //биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
    glBindTexture(GL_TEXTURE_2D, texId);

    //загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);


    //отчистка памяти
    free(texCharArray);
    free(texarray);

    //наводим шмон
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                                                              // (Н) И досюда


    //массив трехбайтных элементов  (R G B)
    RGBTRIPLE* texarray2;

    //массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
    char* texCharArray2;
    int texW2, texH2;
    OpenGL::LoadBMP("texture1.bmp", &texW2, &texH2, &texarray2);
    OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);


    //генерируем ИД для текстуры
    glGenTextures(1, &texId2);
    //биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
    glBindTexture(GL_TEXTURE_2D, texId2);

    //загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);


    //отчистка памяти
    free(texCharArray2);
    free(texarray2);

    //наводим шмон
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    //камеру и свет привязываем к "движку"
    ogl->mainCamera = &camera;
    ogl->mainLight = &light;

    // нормализация нормалей : их длины будет равна 1
    glEnable(GL_NORMALIZE);

    // устранение ступенчатости для линий
    glEnable(GL_LINE_SMOOTH);


    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

    camera.fi1 = -1.3;
    camera.fi2 = 0.8;
}



GLdouble* crossProduct(const double* A, const double* B, const double* D) {
    GLdouble* result = new GLdouble[3];

    // Calculate the components of vector AB
    GLdouble AB_x = B[0] - A[0];
    GLdouble AB_y = B[1] - A[1];
    GLdouble AB_z = B[2] - A[2];

    // Calculate the components of vector DC
    GLdouble CD_x = D[0] - A[0];
    GLdouble CD_y = D[1] - A[1];
    GLdouble CD_z = D[2] - A[2];

    // Calculate the cross product components
    result[0] = -(AB_y * CD_z - AB_z * CD_y);
    result[1] = -(AB_z * CD_x - AB_x * CD_z);
    result[2] = -(AB_x * CD_y - AB_y * CD_x);

    return result;
}


void Render(OpenGL* ogl)
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);
    if (textureMode)
        glEnable(GL_TEXTURE_2D);

    if (lightMode)
        glEnable(GL_LIGHTING);

    if (textureReplace)                                                     // (Н) Здесь переключаются текстуры: textureReplace - создаётся на 20 строчке
        glBindTexture(GL_TEXTURE_2D, texId);                                // (Н) Назначение кнопки на 225
    else
        glBindTexture(GL_TEXTURE_2D, texId2);


    //альфаналожение
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    glDisable(GL_BLEND);


    //настройка материала
    GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
    GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
    GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
    GLfloat sh = 0.1f * 256;


    //фоновая
    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    //дифузная
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    //зеркальная
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    //размер блика
    glMaterialf(GL_FRONT, GL_SHININESS, sh);

    //чтоб было красиво, без квадратиков (сглаживание освещения)
    glShadeModel(GL_SMOOTH);
    //===================================
    //Прогать тут                     (Н) это обман, пришлось прогать не только тут


    //Начало рисования квадратика станкина
                        // (Н) нормали для света я считал вручную, алгоритм по нахождению нормали писал только в циклах!                     
    double A[] = { -2,-1,0 };
    double B[] = { -8, 2, 0 };
    double C[] = { 1,1,0 };
    double D[] = { 2,5,0 };
    double E[] = { 8,5,0 };
    double F[] = { 2,0,0 };
    double G[] = { 4,-4,0 };
    double H[] = { -2,-5,0 };

    double A1[] = { -2,-1,5 };
    double B1[] = { -8, 2, 5 };
    double C1[] = { 1,1,5 };
    double D1[] = { 2,5,5 };
    double E1[] = { 8,5,5 };
    double F1[] = { 2,0,5 };
    double G1[] = { 4,-4,5 };
    double H1[] = { -2,-5,5 };
    GLdouble* Normalek = nullptr;



    glColor3d(0.7, 0.7, 0.7);



    glBegin(GL_POLYGON);
    Normalek = crossProduct(A, B, C);
    glNormal3d(-Normalek[0], -Normalek[1], -Normalek[2]);
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3dv(A);
    glVertex3dv(B);
    glVertex3dv(C);
    glVertex3dv(F);
    glEnd();

    glBegin(GL_POLYGON);
    Normalek = crossProduct(C, D, E);
    glNormal3d(-Normalek[0], -Normalek[1], -Normalek[2]);
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3dv(C);
    glVertex3dv(D);
    glVertex3dv(E);
    glVertex3dv(F);
    glEnd();

    glBegin(GL_POLYGON);
    Normalek = crossProduct(F, G, H);
    glNormal3d(-Normalek[0], -Normalek[1], -Normalek[2]);
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3dv(F);
    glVertex3dv(G);
    glVertex3dv(H);
    glVertex3dv(A);
    glEnd();



    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(A, B, B1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(A);
    glTexCoord2d(0, 1);
    glVertex3dv(B);
    glTexCoord2d(1, 1);
    glVertex3dv(B1);
    glTexCoord2d(1, 0);
    glVertex3dv(A1);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(B, C, C1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(B);
    glTexCoord2d(0, 1);
    glVertex3dv(C);
    glTexCoord2d(1, 1);
    glVertex3dv(C1);
    glTexCoord2d(1, 0);
    glVertex3dv(B1);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(C, D, D1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(C);
    glTexCoord2d(0, 1);
    glVertex3dv(D);
    glTexCoord2d(1, 1);
    glVertex3dv(D1);
    glTexCoord2d(1, 0);
    glVertex3dv(C1);
    glEnd();




    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(E, F, F1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(E);
    glTexCoord2d(0, 1);
    glVertex3dv(F);
    glTexCoord2d(1, 1);
    glVertex3dv(F1);
    glTexCoord2d(1, 0);
    glVertex3dv(E1);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(F, G, G1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(F);
    glTexCoord2d(0, 1);
    glVertex3dv(G);
    glTexCoord2d(1, 1);
    glVertex3dv(G1);
    glTexCoord2d(1, 0);
    glVertex3dv(F1);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(G, H, H1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(G);
    glTexCoord2d(0, 1);
    glVertex3dv(H);
    glTexCoord2d(1, 1);
    glVertex3dv(H1);
    glTexCoord2d(1, 0);
    glVertex3dv(G1);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.3f, 0.3f);
    Normalek = crossProduct(H, A, A1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glTexCoord2d(0, 0);
    glVertex3dv(H);
    glTexCoord2d(0, 1);
    glVertex3dv(A);
    glTexCoord2d(1, 1);
    glVertex3dv(A1);
    glTexCoord2d(1, 0);
    glVertex3dv(H1);
    glEnd();


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    glColor4d(1, 0, 0, 0.5);

    // крышку 
    glBegin(GL_POLYGON);
    Normalek = crossProduct(A1, B1, C1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glColor3f(0.5f, 0.5f, 0.5f);

    glVertex3dv(A1);

    glVertex3dv(B1);

    glVertex3dv(C1);

    glVertex3dv(F1);
    glEnd();

    glBegin(GL_POLYGON);
    Normalek = crossProduct(C1, D1, E1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glColor3f(0.5f, 0.5f, 0.5f);
    glTexCoord2d(0, 0);
    glVertex3dv(C1);

    glVertex3dv(D1);

    glVertex3dv(E1);

    glVertex3dv(F1);
    glEnd();


    glBegin(GL_POLYGON);
    Normalek = crossProduct(A1, F1, G1);
    glNormal3d(Normalek[0], Normalek[1], Normalek[2]);
    glColor3f(0.5f, 0.5f, 0.5f);
    glTexCoord2d(0, 0);
    glVertex3dv(A1);

    glVertex3dv(F1);

    glVertex3dv(G1);

    glVertex3dv(H1);
    glEnd();

    glDisable(GL_BLEND);



    //выпуклрсть

    double x0 = E[0];
    double y0 = E[1];

    double tx0 = 0;
    double ty0 = 0;


    double Det[] = { 0,0,0 };
    double Det1[] = { 0,0,5 };
    double N[] = { 0,0,0 };
    double Nl;

    for (double i = -2.05; i <= -0.95; i += 0.0001)
    {
        double x = 5.0 + 6.08276253 / 2 * cos(i * 3.141593);
        double y = 5.5 + 6.08276253 / 2 * sin(i * 3.141593);

        double tx = tx0 + 1.0 / 7000.0;
        double ty = ty0 + 1.0 / 7000.0;

        glBegin(GL_POLYGON);
        glNormal3d(0, 0, -1);
        glColor3d(0.5f, 0.5f, 0.5f);
        glVertex3dv(D);
        glVertex3d(x0, y0, 0);
        glVertex3d(x, y, 0);
        glEnd();


        //отрисовка верхней части выпуклости
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
        glColor4d(1, 0, 0, 0.5);
        glBegin(GL_POLYGON);
        glNormal3d(0, 0, 1);
        glColor3d(0.5f, 0.5f, 0.5f);
        glVertex3dv(D1);
        glVertex3d(x0, y0, 5);
        glVertex3d(x, y, 5);
        glEnd();
        glDisable(GL_BLEND);


        glBegin(GL_QUADS);
        glColor3d(0.3f, 0.3f, 0.3f);
        Det[0] = { x - (5.0 + 6.08276253 / 2 * cos((i - 0.0001) * 3.141593)) };
        Det[1] = { y - (5.5 + 6.08276253 / 2 * sin((i - 0.0001) * 3.141593)) };
        N[0] = { Det[1] * Det1[2] - Det1[1] * Det[2] };
        N[1] = { -Det[0] * Det1[2] + Det1[0] * Det[2] };
        N[2] = { Det[0] * Det1[1] - Det1[0] * Det[1] };
        Nl = sqrt(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
        N[0] = { N[0] / Nl };
        N[1] = { N[1] / Nl };
        N[2] = { N[2] / Nl };
        glNormal3d(N[0], N[1], N[2]);

        glTexCoord2d(tx0, 0);
        glVertex3d(x0, y0, 0);
        glTexCoord2d(tx, 0);
        glVertex3d(x, y, 0);
        glTexCoord2d(ty, 1);
        glVertex3d(x, y, 5);
        glTexCoord2d(ty0, 1);
        glVertex3d(x0, y0, 5);

        glEnd();

        x0 = x;
        y0 = y;
        tx0 = tx;
        ty0 = ty;
    }






    glMatrixMode(GL_PROJECTION);    //Делаем активной матрицу проекций. 
    //(всек матричные операции, будут ее видоизменять.)
    glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек                     
    glLoadIdentity();     //Загружаем единичную матрицу
    glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);  //врубаем режим ортогональной проекции

    glMatrixMode(GL_MODELVIEW);     //переключаемся на модел-вью матрицу
    glPushMatrix();           //сохраняем текущую матрицу в стек (положение камеры, фактически)
    glLoadIdentity();         //сбрасываем ее в дефолт

    glDisable(GL_LIGHTING);



    GuiTextRectangle rec;          //классик моего авторства для удобной работы с рендером текста.
    rec.setSize(300, 200);
    rec.setPosition(10, ogl->getHeight() - 200 - 10);


    std::stringstream ss;
    ss << "T - вкл/выкл текстур" << std::endl;
    ss << "E - Переключение текстур" << std::endl;
    ss << "L - вкл/выкл освещение" << std::endl;
    ss << "F - Свет из камеры" << std::endl;
    ss << "G - двигать свет по горизонтали" << std::endl;
    ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
    ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
    ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
    ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
    ss << "UV-развёртка" << std::endl;

    rec.setText(ss.str().c_str());
    rec.Draw();

    glMatrixMode(GL_PROJECTION);      //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
    glPopMatrix();


    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

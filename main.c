//Acest proiect este destinat criptarii si decriptarii de imagini in format BitMap, citindu-le ca fisiere binare.
//Formatul BitMap acceptat de acest program este unul simplist, tratand doar imaginile ce contin in scrierea binara cei 54 de octeti de header si tabloul de pixeli.
//Datele de intrare vor fi dupa cum urmeaza:
//                      calea fisierului de intrare;
//                      calea si noul nume al fisierului de iesire (nu este necesar ca acesta sa existe, programul il creeaza automat atat timp cat calea si denumirea sunt date);
//                      calea fisierului text in care se vor afla doua chei secrete, folosite la criptarea unei imagini sau decriptarea unei imagini deja criptate.
//Datele vor fi citite de la tastatura.


#include <stdio.h>
#include <stdlib.h>

//structura ce va contine octetii red, green, blue ai fiecarui pixel
typedef struct pixel
{
    unsigned char r,g,b;
};

//structura ce va permite accesarea octetilor cheilor secrete date ca numar intreg, conform principiului Little Endian
typedef union secretkey
{
    unsigned long int SV;
    unsigned char c[4];
};
//Generatorul XORSHIFT32 etse un generator de numere aparent aleatoare; numerele generate de acesta vor fi folosite la criptare.
//primeste ca parametru un pointer catre un vector alocat dinamic, lungimea acestuia si un "seed" de la care se va incepe generarea numerelor
void XORSHIFT32( unsigned long int *v, long int n, long unsigned int seed)
{
    unsigned  long int nr;
    int i;
    nr=seed;
    //primul numar va fi seed
    v[0]=seed;
    for(i=1; i<n; i++)
    {
        //toate celelalte numere se vor obtine prin shiftarea pixelilor numarului anterior astfel:
        nr=nr^nr<<13;
        nr=nr^nr>>17;
        nr=nr^nr<<5;
        v[i]=nr;
    }
}

//functie pentru citirea Header-ului imaginii
//functia primeste calea fisierului BitMap si, prin intermediul celorlalti parametri, va transmite headerul alocat dinamic, dar si dimensiunile imaginii dupa cum reies din acesta.
void citesc_header(char *calefisier,unsigned char **header, unsigned long int *lung, unsigned long int *lat)
{
    unsigned char h;
    //deschiderea fisierului in format binar
    FILE * f=fopen(calefisier,"rb");
    if(f==NULL)
    {
        printf("Nu am gasit fisierul :(\n");
        exit(0);
    }

    int i;
    //dimensiunile imaginii:
    unsigned long int lungime, latime;
    //alocam dinamic headerul
    (*header)=(unsigned char *)malloc(55*sizeof(unsigned char));

    if(header==NULL)
    {
        printf("Nu am putut aloca headerul.\n");
        exit(0);
    }
    for(i=0; i<18; i++)
    {
        fread(&h, 1, 1, f);
        (*header)[i]=h;
    }
    //urmatorii 8 octeti reprezinta dimensiunile imaginii, prin urmare ii citim separat
    fread(&latime,4,1,f);
    fread(&lungime,4,1,f);

    //reveire pentru a adauga toti octetii in header (inclusiv dimensiunile)
    fseek(f,-8,SEEK_CUR);

    for(i=18; i<54; i++)
    {
        fread(&h, 1, 1, f);
        (*header)[i]=h;
    }
    (*header)[54]='\0';

    //actualizarea dimensiunilor
    (*lung)=lungime;
    (*lat)=latime;
    fclose(f);
}

//o functie ce aloca o matrice de pixeli in varianta liniarizata (ca si vector)
void matrice_pixeli_liniarizata(struct pixel **pix, unsigned long int lungime, unsigned long int latime )
{
    (*pix)=(struct pixel *)malloc(sizeof(struct pixel)*lungime*latime);
    if((*pix)==NULL)
    {
        printf("Nu am putut aloca imaginea.");
        exit(0);
    }
}
//functie pentru citirea tabloului de pixeli si aducerea acestuia la forma liniarizata, asa cum se poate vedea in imagine cu ochiul liber (reprezentand incepand cu coltul de stanga sus si nu stanga jos)
void citire_pixel(char *calefisier, struct pixel *img, unsigned long int lungime, unsigned long int latime)
{
    FILE *fin=fopen(calefisier,"rb");

    //se trece peste header
    fseek(fin, 54, SEEK_SET);
    unsigned char r,g,b;
    int i,j;
    struct pixel aux;
    unsigned char x;
    //citirea
    for(i=0; i<lungime*latime; i++)
    {

        {
            fread(&b,1,1,fin);
            fread(&g,1,1,fin);
            fread(&r,1,1,fin);
            img[i].b=b;
            img[i].g=g;
            img[i].r=r;
        }
        if((i+1)%latime==0)
        {
          //verificare daca exista octeti de padding
            if(latime%4!=0)
            {
                for(j=1; j<=4-(latime*3)%4; j++)
                {
                    fread(&x, 1,1,fin);
                }
            }

        }
    }
    //printf("%d %d %d", img[0].r, img[0].g,img[0].b);

    //interschimbarea liniilor
    for(i=0; i<lungime/2; i++)
    {
        for(j=0; j<latime; j++)
        {
            aux=img[i*latime+j];
            img[i*latime+j]=img[(lungime-1-i)*latime+j];
            img[(lungime-1-i)*latime+j]=aux;
        }
    }

    fclose(fin);
}
//afisarea unei imagini stocate sub forma liniarizata
void afisare(char *calefisier,unsigned char *header, struct pixel *img, unsigned long int lungime, unsigned long int latime)
{
    FILE *fout=fopen(calefisier, "wb");
    int i,j;
    struct pixel aux;
    unsigned char x=0;
    printf("\n");
    //scrierea headerului
    for(i=0; i<54; i++)
    {
        fwrite(&header[i], 1,1,fout);

    }
    //interschimbarea liniilor

    for(i=0; i<lungime/2; i++)
    {
        for(j=0; j<latime; j++)
        {
            aux=img[i*latime+j];
            img[i*latime+j]=img[(lungime-1-i)*latime+j];
            img[(lungime-1-i)*latime+j]=aux;
        }
    }
    //scrierea tabloului de pixeli
    for(i=0; i<lungime; i++)
    {


        for(j=0; j<latime; j++)
        {
            fwrite(&img[i*latime+j].b,1,1,fout);
            fwrite(&img[i*latime+j].g,1,1,fout);
            fwrite(&img[i*latime+j].r,1,1,fout);
        }

         //padding
            if(latime%4!=0)
            {
                for(j=1; j<=4-(latime*3)%4; j++)
                    fwrite(&x, 1,1,fout);

            }

        }

    fclose(fout);
}

//functie ce creeaza o permutarea aparent aleatoare, folosind un vector de numere generate aleator
unsigned long int * PermutareAleatoare(unsigned long int n, unsigned long int *random)
{
    //alocam permutarea
    unsigned long int *Permutare=(unsigned long int *)malloc(n*sizeof(unsigned long int));
    if(Permutare==NULL)
    {
        printf("Nu am putut aloca permutarea.");
        exit(0);
    }
    unsigned long int aux,r;
    unsigned long int i;
     //initializam permutarea
    for(i=0; i<n; i++)
        Permutare[i]=i;
    //cream permutarea in functie de vectorul de numere aleatoare
    for(i=n-1; i>=1; i--)
    {
        r=random[n-i]%(i+1);
        aux=Permutare[r];
        Permutare[r]=Permutare[i];
        Permutare[i]=aux;
    }

    return Permutare;
}
//functie ce aplica o permutare unui tablou de pixeli
void CreeazaPermutare(struct pixel *v, unsigned long int *permutare, unsigned long int n)
{
    int i;
    //folosim un vector auxiliar
    struct pixel *a=(struct pixel *)malloc(n*sizeof(struct pixel)),aux;
    for(i=0; i<n; i++)
    {
        a[permutare[i]]=v[i];
    }

    for(i=0; i<n; i++)
    {
        v[i]=a[i];
    }
    free(a);

}

//Functia de criptare ce primeste ca parametri caile fisierelor si creeaza noua imaginr criptata
void criptare (char *calefisierintrare, char *calefisieriesire, char *calecheie)
{

    FILE * Fsecret;
    Fsecret=fopen(calecheie, "r");

    unsigned long int cheie_criptare_1, cheie_criptare_2,*permutare,*randomy;
    fscanf(Fsecret,"%lu %lu", &cheie_criptare_1, &cheie_criptare_2);

    unsigned long int lungime, latime,marime;
    unsigned char *header;
    struct pixel *img;

    //citire headerul
    citesc_header(calefisierintrare,&header,&lungime, &latime);

    //alocare tabloul de pixeli
    matrice_pixeli_liniarizata(&img,lungime,latime);

    //citire tabloul
    citire_pixel(calefisierintrare,img, lungime,latime);
    marime=lungime*latime;

    //creare de vector pentru numrerele aleatoare
    randomy=(unsigned long int*)malloc((2*marime)*sizeof(unsigned long int));

    //generare de numerele aleatoare folosind XORSHIFT32
    XORSHIFT32(randomy,2*marime,cheie_criptare_1);

    //creare a unei permutari pe baza acelor numere
    permutare=PermutareAleatoare(marime, randomy);

    //aplicarea permutarii pe matricea de pixeli
    CreeazaPermutare(img,permutare,marime);

    //dupa permutare, se vor duce toate canalele de culoare la valori aproximativ omogene, folosind cel de-al doilea numar din fisierul cheii secrete
    int i;
    union secretkey secret,nr_random;
    secret.SV=cheie_criptare_2;
    nr_random.SV=randomy[marime];

    //se va efectua operatia XOR intre octeti de pe pozitiile 0,1,2 in scrierea LittleEndian a cheii +primului nr. random si octetii de culoare astfel:
    img[0].r=(secret.c[2])^(img[0].r)^nr_random.c[2];
    img[0].g=(secret.c[1])^(img[0].g)^nr_random.c[1];
    img[0].b=(secret.c[0])^(img[0].b)^nr_random.c[0];
    for(i=1; i<marime; i++)
    {   //se repeta procedeul, "xorand" in plus cu numerele generate random
        nr_random.SV=randomy[marime+i];
        img[i].r=(img[i-1].r)^(img[i].r)^nr_random.c[2];
        img[i].g=(img[i-1].g)^(img[i].g)^nr_random.c[1];
        img[i].b=(img[i-1].b)^(img[i].b)^nr_random.c[0];
    }
    //afisam
    afisare(calefisieriesire,header,img, lungime,latime);

    free(randomy);
    fclose(Fsecret);
    free(img);
    free(header);
    free(permutare);
}

//Partea de decriptare
//Decriptarea este simetrica criptarii, repetand procesele in ordine inversa

//functie ce genereaza perutarea inversa unei permutari date
void permutareinversa( unsigned long int *permutare,unsigned long int n)
{
    unsigned long int i, *inversa;
    inversa=(unsigned int *)malloc(n*sizeof(unsigned int));
    for(i=0; i<n; i++)
    {
       inversa[permutare[i]]=i;
    }
     for(i=0;i<n;i++)
     {
         permutare[i]=inversa[i];
     }
    free(inversa);

}

//functia de decriptare
void decriptare (char *calefisierintrare, char *calefisieriesire, char *calecheie)
{
    FILE * Fsecret;
    Fsecret=fopen(calecheie, "r");
    unsigned long int cheie_criptare_1, cheie_criptare_2,*permutare,*randomy;
    union secretkey secret,nr_random;
    fscanf(Fsecret,"%lu %lu", &cheie_criptare_1, &cheie_criptare_2);
    secret.SV=cheie_criptare_2;

    unsigned long int lungime, latime,marime;
    unsigned char *header;
    struct pixel *img;

    //citim imaginea data si o alocam dinamic, retinand lungimea, latimea si headerul
    citesc_header(calefisierintrare,&header,&lungime, &latime);
    matrice_pixeli_liniarizata(&img,lungime,latime);
    citire_pixel(calefisierintrare,img, lungime,latime);
    //copiem imaginea pentru criptare
    marime=lungime*latime;

    //generarea numerelor aleatoare
    randomy=(unsigned long int*)malloc((2*marime+1)*sizeof(unsigned long int));

    XORSHIFT32(randomy,2*marime+1,cheie_criptare_1);
    //generarea permutarii
    permutare=PermutareAleatoare(marime, randomy);

    unsigned long int i;

    //repetarea operatiei XOR in exact aceeasi ordine pentru a aduce pixelii la forma initiala
    for(i=marime;i>=1;i--)
    {
    nr_random.SV=randomy[marime+i];
    img[i].r=img[i-1].r^img[i].r^nr_random.c[2];
    img[i].g=img[i-1].g^img[i].g^nr_random.c[1];
    img[i].b=img[i-1].b^img[i].b^nr_random.c[0];
    }
    nr_random.SV=randomy[marime];
    img[0].r=secret.c[2]^img[0].r^nr_random.c[2];
    img[0].g=secret.c[1]^img[0].g^nr_random.c[1];
    img[0].b=secret.c[0]^img[0].b^nr_random.c[0];

    //aplicarea permutarii inverse pentru a aduce pixelii l ordinea corecta
    permutareinversa(permutare,marime);
    //aplicam permutarea pe pixeli
    CreeazaPermutare(img, permutare,marime);

    //eliberam memoria si inchidem fisierele
    afisare(calefisieriesire,header,img,lungime,latime);
    free(permutare);
    free(randomy);
    free(img);
    free(header);
    fclose(Fsecret);
}
//functie de copiere a doi vectori de pixeli
void copiere (struct pixel *v1, struct pixel *v2, unsigned long int marime)
{
    int i;
    for(i=0; i<marime; i++)
        v2[i]=v1[i];
}
//testu chi^2 ne ofera valorile medii ale pixelilor pe fiecare canal de culoare, pentru a verifica ca imaginea criptata este omogena
void TestulChiPatrat(char *calefisier)
{

    unsigned long int lungime, latime,marime;
    unsigned char *header;
    struct pixel *img;

    //citim imaginea data si o alocam dinamic, retinand lungimea, latimea si headerul
    citesc_header(calefisier,&header,&lungime, &latime);
    matrice_pixeli_liniarizata(&img,lungime,latime);
    citire_pixel(calefisier,img, lungime,latime);
    marime=lungime*latime;

    unsigned long int i,j;
    unsigned long int **frecventa;
    //matrice pentru frecventa medie a fiecarei culori in imagine

    frecventa=(unsigned long int **)malloc(3*sizeof(unsigned long int *));
    for(i=0;i<3;i++)
        frecventa[i]=(unsigned int *)malloc(256*sizeof(unsigned long int));
    for(i=0; i<3; i++)
        for(j=0; j<256; j++)
            frecventa[i][j]=0;

    //media pixelilor
    double frecventaMedie=(float)marime/256;
    double suma[3]= {0};
    unsigned int x;
    //update frecventa
    for(i=0; i<marime; i++)
    {
        x=(unsigned int)img[i].b;
        frecventa[0][x]++;
        x=(unsigned int)img[i].g;
        frecventa[1][x]++;
        x=(unsigned int)img[i].r;
        frecventa[2][x]++;
    }
    //calculam deviatia standard pe fiecare canal de culoare
    for(i=0; i<256; i++)
    {
        double fr=(frecventa[0][i]-frecventaMedie)*(frecventa[0][i]-frecventaMedie);
        fr=(float)fr/frecventaMedie;
        suma[0]=suma[0]+fr;
        fr=(frecventa[1][i]-frecventaMedie)*(frecventa[1][i]-frecventaMedie);
        fr=(float)fr/frecventaMedie;
        suma[1]=suma[1]+fr;
        fr=(frecventa[2][i]-frecventaMedie)*(frecventa[2][i]-frecventaMedie);
        fr=(float)fr/frecventaMedie;
        suma[2]=suma[2]+fr;
    }
    double sumatotala=suma[0]+suma[1]+suma[2];
    printf("Canalul albastru:%lf\nCanalul verde:%lf\nCanalul rosu:%lf\n ", suma[0],suma[1],suma[2]);

    for(i=0;i<3;i++)
    free(frecventa[i]);
    free(frecventa);
}
int main()
{

    //accesati meniul
    printf("Buna ziua! Introduceti calea imaginii pe care doriti sa prelucrati: ");

    //denumirea fisierului de intrare
    char *calefisierintrare=(char *)malloc(220*sizeof( char));
    fgets (calefisierintrare,220, stdin);
    calefisierintrare[strlen(calefisierintrare)-1]='\0';
    //denumire fisier de iesire
    char *calefisieriesire=(char *)malloc(220*sizeof( char));
    printf("\nIntroduceti calea si denumirea noului fisier creat: ");
    fgets (calefisieriesire,220, stdin);
    calefisieriesire[strlen(calefisieriesire)-1]='\0';

    char *calecheie=(char *)malloc(220*sizeof( char));
    printf("\nIntroduceti calea fisierului cu cheia secreta: ");
    fgets (calecheie,220, stdin);
    calecheie[strlen(calecheie)-1]='\0';

    printf("Ce doriti sa ii facem imaginii de intrare? Selectati numarul pentru comanda dorita:\n1. Criptare + Testul Chi pentru imaginea criptata\n2.Decriptare + Testul Chi pentru imaginea decriptata\n");
    int comanda;
    scanf("%d", &comanda);
    if(comanda==1)criptare(calefisierintrare, calefisieriesire, calecheie);
    else
    decriptare(calefisierintrare,calefisieriesire,calecheie);
    //afisare(calefisieriesire,header,rigacripto, lungime,latime);

    TestulChiPatrat(calefisieriesire);

    free(calecheie);
    free(calefisieriesire);
    free(calefisierintrare);


    return 0;
}

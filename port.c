#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#define REZERWA 500
//definicja stanow statku
#define PORT 1
#define START 2
#define REJS 3
#define KONIEC_REJSU 4
#define KATASTROFA 5
#define TANKUJ 5000
int paliwo = 5000;
int LADUJ=1, NIE_LADUJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_statkow;
int ilosc_portow=4;
int ilosc_zajetych_portow=0;
int tag=1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;

void Pasazer (){
printf("Pasażer za burtą\n");
if(rand()%2==0)
{
printf("Udało się. Pasażer uratowany.\n");
}
else
{
printf("Pasażer utonął.\n");
}       
}

void Fale (){
if(rand()%3==0)
{
printf("UWAGA pasazerowie, zrywa sie wiatr. Bedzie mocno bujalo\n");
}
else
{
printf("Wody spokojne.\n");
}              
}

void Wyslij(int nr_statku, int stan)
{
wyslij[0]=nr_statku;
wyslij[1]=stan;
MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
sleep(1);
}

void Port(int liczba_procesow){
int nr_statku,status;
ilosc_statkow = liczba_procesow -1;
printf("Halo, Witam serdecznie na pokładzie statku\n");
if(rand()%2==1){
printf("Mamy piekna pogode sprzyjajaca rejsom\n");
}
else{
printf("Niestety pogoda nie sprzyja dzisiejszym rejsom.");
}
printf("Zyczymy Panstwu, przyjemnej podrozy \n \n\n");
sleep(2);

while(ilosc_portow<=ilosc_statkow){
MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
nr_statku=odbierz[0];
status=odbierz[1];

if(status==1){
printf("Statek %d stoi w porcie.\n", nr_statku);
}
if(status==2){
printf("Statek %d pozwolenie na wypłyniecie z portu nr %d\n", nr_statku, ilosc_zajetych_portow);
ilosc_zajetych_portow--;
}
if(status==3){
printf("Statek %d PLYWA\n", nr_statku);
    if(rand()%2==1)
    {
        Fale();
        Pasazer();                        
    }
}
if(status==4){
    if(ilosc_zajetych_portow<ilosc_portow){
    ilosc_zajetych_portow++;
    MPI_Send(&LADUJ, 1, MPI_INT, nr_statku, tag, MPI_COMM_WORLD);
    }
    else{
    MPI_Send(&NIE_LADUJ, 1, MPI_INT, nr_statku, tag, MPI_COMM_WORLD);
    }
}
if(status==5){
    ilosc_statkow--;
    printf("Ilosc statkow %d\n", ilosc_statkow);
}
}

printf("Program zakonczyl dzialanie:)\n");
}

void Statek(){
int  stan,suma,i;
stan=REJS;
while(1){
if(stan==1){
    if(rand()%2==1){
        stan=START;
        paliwo=TANKUJ;
        printf("Prosze o pozwolenie na wyplyniecie, statek %d\n",nr_procesu);
        Wyslij(nr_procesu,stan);
    }
    else{
    Wyslij(nr_procesu,stan);
    }
}
else if(stan==2){
    printf("Wyplynalem, statek %d\n",nr_procesu);
    stan=REJS;
    Wyslij(nr_procesu,stan);
}
else if(stan==3){
    paliwo-=rand()%500;
    if(paliwo<=REZERWA){
    stan=KONIEC_REJSU;
    printf("Prosze o pozwolenie na zawiniecie do portu\n");
    Wyslij(nr_procesu,stan);
    }
    else{
    for(i=0; rand()%10000;i++); 
    }
}
else if(stan==4){
    int temp;
    MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
    if(temp==LADUJ){
    stan=PORT;
    printf("Zawinalem do portu, statek %d\n", nr_procesu);
    }
    else
    {
    paliwo-=rand()%500;
    
    if(paliwo>0){
    Wyslij(nr_procesu,stan);
    }
    else{
    stan=KATASTROFA;
    printf("KATASTROFA\n");
    Wyslij(nr_procesu,stan);
    return; 
    }
}
}
}
}

int main(int argc, char *argv[])
{
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
srand(time(NULL));
if(nr_procesu == 0) 
    Port(liczba_procesow);
else 
    Statek();              
MPI_Finalize();
return 0;
}

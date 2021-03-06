#include <car.h>

uint32_t start_tick,dist_tick;

void car_init()
{
    FLAG_CAR = 0;
    FLAG_SENSOR = 1;

    gpioSetMode(GPIN23,PI_OUTPUT); gpioSetMode(GPIN24,PI_OUTPUT);
    gpioSetMode(GPIN25,PI_OUTPUT); 
    gpioSetMode(GPIN6,PI_OUTPUT);  gpioSetMode(GPIN13,PI_OUTPUT);
    gpioSetMode(GPIN19,PI_OUTPUT);

    gpioWrite(GPIN25,1); // Left Enable
    gpioWrite(GPIN19,1); // Right Enable
}
void sensor_init()
{
    pthread_t thr;
    int thr_id;
    
    gpioCfgClock(2, 1, 1);
    gpioSetMode(GPIN2,PI_OUTPUT);
    gpioSetMode(GPIN3,PI_INPUT);

    gpioSetAlertFunc(GPIN3,sensor_rising_func);
    gpioWrite(GPIN2,PI_OFF);
    gpioDelay(1000000);

    thr_id = pthread_create(&thr,NULL,sensor_thread,NULL);
}
void* sensor_thread(void* data)
{
    while(1)
    {
        while(FLAG_SENSOR) {
        start_tick = dist_tick = 0;
        gpioTrigger(GPIN2,10,PI_HIGH);
        gpioDelay(50000);
        if(dist_tick && start_tick)
        {
            float distance = dist_tick / 1000000. * 340 / 2 * 100;
            if(distance<50)
            {
                FLAG_CAR = 1;
            }
            else
            {
#ifdef PRINT_SENSOR
                printf("interrval : %6dus, Distance : %6.1f cm\n",dist_tick,distance);
#endif
                FLAG_CAR = 0;
            }
        }
#ifdef PRINT_SENSOR
        else printf("sensor error\n");
#endif
        gpioDelay(100000);
        }
    }

}
void sensor_rising_func(int gpio,int level,uint32_t tick)
{
    if(level == PI_HIGH)
        start_tick = tick;
    else if(level == PI_LOW)
        dist_tick = tick - start_tick;
}
void forward()
{
    gpioWrite(GPIN23, 1); gpioWrite(GPIN24, 0); // Left
    gpioWrite(GPIN6, 1); gpioWrite(GPIN13, 0);  // Right
}
void backward()
{
    gpioWrite(GPIN23, 0); gpioWrite(GPIN24, 1);
    gpioWrite(GPIN6, 0); gpioWrite(GPIN13, 1);
}
void move_right()
{
    gpioWrite(GPIN23, 1); gpioWrite(GPIN24, 0);
    gpioWrite(GPIN6, 0); gpioWrite(GPIN13, 1);
}
void move_left()
{
    gpioWrite(GPIN23, 0); gpioWrite(GPIN24, 1);
    gpioWrite(GPIN6, 1); gpioWrite(GPIN13, 0);
}
void stop_car()
{
    gpioWrite(GPIN23, 0); gpioWrite(GPIN24, 0);
    gpioWrite(GPIN6, 0); gpioWrite(GPIN13, 0);
}

void move_car() {
    while(FLAG_SENSOR) {
       if (FLAG_CAR) {
           stop_car();
           backward();
           time(NULL)%2 ? move_right : move_left();
           while(FLAG_CAR);
           stop_car();
        }
        forward();
    }
}



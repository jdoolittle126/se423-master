package com.jonathandoolittle;

import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

public class Main {

    final static short PAGE_SIZE = 1 << 12;
    final static byte SEQUENCE_SIZE = 100;
    final static byte OPEN_PAGES = 3;


    public static void main(String[] args) {
        VirtualMemorySimulator vm = new VirtualMemorySimulator(PAGE_SIZE, SEQUENCE_SIZE, OPEN_PAGES);

        String pageRefStream = vm.pageReferenceDisplay();
        String beldayResult = vm.beladySimulation();
        String fifoResult = vm.fifoSimulation();
        String lfuResult = vm.lfuSimulation();

        System.out.println(pageRefStream);
        System.out.println(beldayResult);
        System.out.println(fifoResult);
        System.out.println(lfuResult);
    }




}

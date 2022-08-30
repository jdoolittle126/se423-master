package com.jonathandoolittle;

import java.util.*;

public class VirtualMemorySimulator {

    private final Random m_random = new Random();

    // Since this is a memory simulator, we have to
    // save all the memory we can by using the smallest
    // possible data types 👀
    private final short m_pageSize;
    private final byte m_sequenceSize;
    private final byte m_openPages;
    private short m_faults;

    // Characters are TECHNICALLY an unsigned 16 bit int... hehehe
    private final char[] m_pageReferenceStream;
    private byte[] m_virtualMemory;

    public VirtualMemorySimulator(short pageSize, byte sequenceSize, byte openPages) {
        m_pageSize = pageSize;
        m_sequenceSize = sequenceSize;
        m_openPages = openPages;
        m_pageReferenceStream = new char[sequenceSize];
        populateReferenceStream();
    }

    private void populateReferenceStream() {
        for (byte b = 0; b < m_sequenceSize; b++) {
            // Generate a random 16 bit value for our address
            m_pageReferenceStream[b] = (char) m_random.nextInt(1 << 16);
        }
    }

    private byte decodeAddress(char address) {
        byte pageNumber = (byte) (address / m_pageSize);
        short offset = (short) (address % m_pageSize);

        System.out.printf("Decoding address 0x%x\t[Page %d, Offset %d]%n",
                (int) address, pageNumber, offset);

        return pageNumber;
    }

    private void resetSimulation() {
        m_faults = 0;
        m_virtualMemory = new byte[m_openPages];
        Arrays.fill(m_virtualMemory, (byte) -1);
    }

    private String printSimulation(String name) {
        return name + " experienced " + m_faults + " faults over " + m_sequenceSize + " virtual addresses!";
    }

    public String pageReferenceDisplay() {
        StringBuilder sb = new StringBuilder();
        sb.append("Stream: ");
        sb.append('[');
        for (char c: m_pageReferenceStream) {
            sb.append(String.format("0x%x, ", (int) c));
        }
        sb.append(']');
        return sb.toString();
    }

    public String fifoSimulation() {
        resetSimulation();

        Queue<Byte> fifo = new LinkedList<>();

        // I DON'T like repeating code but since these are
        // all simulations, and I wasn't sure what adjustments
        // were going to need to be made, I did it anyways :O
        for (byte s = 0; s < m_sequenceSize; s++) {
            // decode our page #
            byte page = decodeAddress(m_pageReferenceStream[s]);
            boolean isPageLoaded = false;

            for (byte p = 0; p < m_openPages; p++) {
                // If the item is already loaded, we are good!
                if (m_virtualMemory[p] == page) {
                    isPageLoaded = true;
                    break;
                }

                // If we got a blank slot, use it!
                if (m_virtualMemory[p] == -1) {
                    m_virtualMemory[p] = page;
                    fifo.add(p);
                    m_faults++;
                    isPageLoaded = true;
                    break;
                }
            }

            if (!isPageLoaded) {
                byte index = fifo.remove();
                m_virtualMemory[index] = page;
                fifo.add(index);
                m_faults++;
            }
        }
        return printSimulation("FIFO simulation");
    }

    public String lfuSimulation() {
        resetSimulation();

        Map<Byte, Byte> lfuMap = new HashMap<>();

        for (byte b = 0; b < 16; b++) {
            lfuMap.put(b, (byte) 0);
        }

        // I DON'T like repeating code but since these are
        // all simulations, and I wasn't sure what adjustments
        // were going to need to be made, I did it anyways :O
        for (byte s = 0; s < m_sequenceSize; s++) {
            // decode our page #
            byte page = decodeAddress(m_pageReferenceStream[s]);
            boolean isPageLoaded = false;
            byte item = lfuMap.get(page);
            for (byte p = 0; p < m_openPages; p++) {

                // If the item is already loaded, we are good!
                if (m_virtualMemory[p] == page) {
                    isPageLoaded = true;
                    lfuMap.put(page, ++item);
                    break;
                }

                // If we got a blank slot, use it!
                if (m_virtualMemory[p] == -1) {
                    m_virtualMemory[p] = page;
                    m_faults++;
                    lfuMap.put(page, ++item);
                    isPageLoaded = true;
                    break;
                }
            }

            // We need to replace a page
            if (!isPageLoaded) {

                byte lowestValue = Byte.MAX_VALUE;
                byte lowestIndex = 0;

                for (byte p = 0; p < m_openPages; p++) {
                    if (item < lowestValue) {
                        lowestValue = item;
                        lowestIndex = p;
                    } else if (item == lowestValue) {
                        // Our tiebreaker is just a coin toss
                        if (m_random.nextBoolean()) {
                            lowestValue = item;
                            lowestIndex = p;
                        }
                    }
                }

                lfuMap.put(page, ++item);
                // Replace it with our page
                m_virtualMemory[lowestIndex] = page;
                m_faults++;
            }
        }

        return printSimulation("LFU simulation");
    }

    public String beladySimulation() {
        resetSimulation();

        // I DON'T like repeating code but since these are
        // all simulations, and I wasn't sure what adjustments
        // were going to need to be made, I did it anyways :O
        for (byte s = 0; s < m_sequenceSize; s++) {
            // decode our page #
            byte page = decodeAddress(m_pageReferenceStream[s]);
            boolean isPageLoaded = false;

            for (byte p = 0; p < m_openPages; p++) {
                // If the item is already loaded, we are good!
                if (m_virtualMemory[p] == page) {
                    isPageLoaded = true;
                    break;
                }

                // If we got a blank slot, use it!
                if (m_virtualMemory[p] == -1) {
                    m_virtualMemory[p] = page;
                    m_faults++;
                    isPageLoaded = true;
                    break;
                }
            }

            // We need to replace a page
            if (!isPageLoaded) {

                // Find the furthest away page on our stream
                byte farthestIndex = 0;
                byte forIndex = 0;
                for (byte p = 0; p < m_openPages; p++) {
                    for (byte index = s; index < m_sequenceSize; index++) {
                        if (index > farthestIndex) {
                            farthestIndex = index;
                            forIndex = p;
                        }
                        if (decodeAddress(m_pageReferenceStream[index]) == page) {
                            break;
                        }
                    }
                }

                // Replace it with our page
                m_virtualMemory[forIndex] = page;
                m_faults++;
            }
        }

        return printSimulation("Belady's optimal algorithm simulation");
    }
    
}

import java.util.Scanner;

public class Cluster {

    private static int[] extendArray(int[] array){
        int[] newArr = new int[array.length * 2];
        for (int i = 0; i < array.length; i++) {
            newArr[i] = array[i];
        }
        return newArr;
    }
    private static int[] removeNegative(int[] array){
        int i = 0;
        while (i < array.length && array[i] >= 0){
            i++;
        }
        int[] subArray = subArray(array, i);
        return subArray;
    }

    private static int[] subArray(int[] array, int size) {
        int[] subArray = new int[size];
        for (int j = 0; j < size; j++) {
            subArray[j] = array[j];
        }
        return subArray;
    }

    private static void swap(int[] array, int i) {
        int tempI = array[i];
        array[i] = array[i+1];
        array[i+1] = tempI;
    }

    private static int[] sortArray(int[] array){
        boolean hasSwapped = true;
        while(hasSwapped){
            hasSwapped = false;
            for (int i = 0; i < array.length - 1; i++) {
                if(array[i] > array[i+1]){
                    swap(array, i);
                    hasSwapped = true;
                }
            }
        }
        return array;
    }

    public static int[] collectClusters(int[] array){
        if(array.length == 0){
            return new int[0];
        }
        array = sortArray(array);
        int[] clusters = new int[array.length];
        int currCluster = 0;
        int clustersStartI = 0;
        for (int i = 0; i < array.length; i++) {
            // if last number or not [...,n, n+1,...]
            if((i == array.length - 1) || (array[i+1] - array[i]) > 1){
                int median = array[ (clustersStartI + i)/2 ];
                clusters[currCluster] = median;
                currCluster++;
                clustersStartI = i+1;
            }
        }
        clusters = subArray(clusters, currCluster);
        return clusters;
    }

    public static void main(String[] args) {
        int[] array = new int[1];
        int i = 0;
        int current;
        Scanner scanner = new Scanner(System.in);
        do{
            current = scanner.nextInt();
            if(array.length == i+1){
                array = extendArray(array);
            }
            array[i] = current;
            i++;
        }while (current >= 0);
        int clusters[] = collectClusters(removeNegative(array));
        for (int j = 0; j < clusters.length; j++) {
            System.out.println(clusters[j]);
        }
    }
}

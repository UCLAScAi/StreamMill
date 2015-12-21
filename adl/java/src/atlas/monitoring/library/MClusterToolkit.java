/*
 */
package atlas.monitoring.library;


public class MClusterToolkit {

  public static MBinSummaryUI[] merge(MBinSummary newBins, MBinSummary oldBins) {
    int size = //newBins.GetBins().length + oldBins.GetBins().length;
              newBins.GetMaxBinIndex() + oldBins.GetMaxBinIndex() + 2; 
              //+2 because both are max as opposed size
    MBinSummaryUI[] binSummaries = new MBinSummaryUI[size];
    
    MClusterSummary nc = newBins.GetBins()[0];
    binSummaries[0] = new MBinSummaryUI(0, nc.getCount(), false, false);
    
    MClusterSummary oc = oldBins.GetBins()[0];
    binSummaries[1] = new MBinSummaryUI(0, oc.getCount(), true, false);
    
    int binSummariesIndex = 2;
    double distance = 0;
    double distanceX = 0;
    double distanceY = 0;
    
    for(int i = 1; i <= newBins.GetMaxBinIndex(); i ++) {
      newBins.GetBins()[i].setProcessed(false);
    }
    
    for(int i = 1; i <= oldBins.GetMaxBinIndex(); i ++) {
      oldBins.GetBins()[i].setProcessed(false);
    }
    
    //System.out.println("New max " + newBins.GetMaxBinIndex() + ", old max " + oldBins.GetMaxBinIndex());
    
    for(int i = 1; i <= oldBins.GetMaxBinIndex(); i++) {
      MClusterSummary closest = null;
      double distanceToClosest = -1;
      oc = oldBins.GetBins()[i];
      if(oc.getCount() > 0) {
        for(int j = 1; j <= newBins.GetMaxBinIndex(); j++) {
          nc = newBins.GetBins()[j];
          if(nc.isProcessed() == false) {
            distanceX = nc.getAverageX() - oc.getAverageX();
            distanceY = nc.getAverageY() - oc.getAverageY();
            distance = Math.sqrt(Math.pow(distanceX, 2) + Math.pow(distanceY, 2));
            if(distanceToClosest == -1 || distanceToClosest > distance) {
              distanceToClosest = distance;
              closest = nc;
            }
          }
        }
        System.out.println("In distance computation - distance to closest = " + distanceToClosest + " : looking for < " + oc.getMaxDistanceAllowed());
        if(distanceToClosest != -1 && distanceToClosest < oc.getMaxDistanceAllowed()) {
          System.out.println("Got match - " + i + ", binSummariesIndex " + binSummariesIndex);
          binSummaries[binSummariesIndex] = new MBinSummaryUI(i, closest.getCount(), false, false);
          binSummariesIndex++;
          closest.setProcessed(true);
          binSummaries[binSummariesIndex] = new MBinSummaryUI(i, oc.getCount(), true, false);
          binSummariesIndex++;
          oc.setProcessed(true);
        }
      }
    }
    System.out.println("binSummariesIndex " + binSummariesIndex + ", size " + size);
    
    int indexTaken = oldBins.GetMaxBinIndex();
    for(int i = 1; i <= newBins.GetMaxBinIndex(); i++) {
      nc = newBins.GetBins()[i];
      if(nc.isProcessed() == false && nc.getCount() > 0) {
        binSummaries[binSummariesIndex] = new MBinSummaryUI(indexTaken++, nc.getCount(), false, false);
        binSummariesIndex++;
        nc.setProcessed(true);
      }
    }
    

    System.out.println("binSummariesIndex " + binSummariesIndex);
    for(int i = 1; i <= oldBins.GetMaxBinIndex(); i++) {
      oc = oldBins.GetBins()[i];
      if(oc.isProcessed() == false && oc.getCount() > 0) {
        binSummaries[binSummariesIndex] = new MBinSummaryUI(indexTaken++, oc.getCount(), true, true);
        binSummariesIndex++;
        oc.setProcessed(true);
      }
    }
    
    
    //temp
    /*for(int i = 0; i < binSummaries.length; i ++) {
      if(binSummaries[i] == null) {
        System.out.println("Index " + i + ": <null>");
      }
      else {
        System.out.println("Index " + i + ": " + binSummaries[i].getBinIndex() 
                              + ", " + binSummaries[i].getValue()
                              + ", " + binSummaries[i].isOld());
      }
    }*/
    
    return binSummaries;
  }

}

package crawltwitter;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.TimeUnit;
import java.util.List;

import java.awt.geom.Path2D;
import java.awt.geom.PathIterator;


public final class UsaMap {
  public static void main(String[] args) {
    try {
      Init();
      _Test();
    } catch (Exception e) {
      e.printStackTrace();
      System.out.println("Exception caught: " + e);
      System.exit(1);
    }
  }

  public static void Init() throws java.io.FileNotFoundException, java.io.IOException {
    _Load();
  }

  private static void _Test() {
    ThreadLocalRandom tlr = ThreadLocalRandom.current();

    int num_points = 10000;
    System.out.printf("Testing with %d random points in the bounding box of the contiguous USA ...\n", num_points);

    int num_contains = 0;
    int num_not_contains = 0;
    long t0 = System.nanoTime();

    for (int i = 0; i < num_points; i ++) {
      // https://www.quora.com/What-is-the-longitude-and-latitude-of-a-bounding-box-around-the-continental-United-States
      double lon = tlr.nextDouble(-124.848974, -66.885444);
      double lat = tlr.nextDouble(24.396308, 49.384358);

      // 10,000 point test:
      //   Without bounding box test: 2010 ms.
      //   With bounding box test   :   94 - 114 ms.
      //   With bounding box test and my own polygon class : 93 - 125 ms. Not worth the effort. Keep the code for portability.
      // Random doubles generation overhead is negligible: 3 ms.
      if (Contains(lon, lat))
        num_contains ++;
      else
        num_not_contains ++;
    }
    long t1 = System.nanoTime();
    System.out.printf("%f%% (%d / %d) points are in the USA. %d ms."
        , 100.0 * num_contains / num_points
        , num_contains
        , num_points
        , TimeUnit.NANOSECONDS.toMillis(t1 - t0)
        );
  }

  //static class Polygon {
  //  List<Double> lons;
  //  List<Double> lats;

  //  Polygon(List<Double> lons_, List<Double> lats_) {
  //    lons = new ArrayList();
  //    lats = new ArrayList();
  //    for (Double p: lons_)
  //      lons.add(p);
  //      //lons.add(p.clone());
  //    for (Double p: lats_)
  //      lats.add(p);
  //      //lats.add(p.clone());
  //  }

  //  public boolean contains(double x, double y) {
  //    int i;
  //    int j;
  //    boolean result = false;
  //    for (i = 0, j = lons.size() - 1; i < lons.size(); j = i++) {
  //      if ( ((lats.get(i) > y) != (lats.get(j) > y))
  //          && (x < (lons.get(j) - lons.get(i)) * (y - lats.get(i)) / (lats.get(j)-lats.get(i)) + lons.get(i)) ) {
  //        result = ! result;
  //      }
  //    }
  //    return result;
  //  }
  //}

  static class BB {
    double x0, x1, y0, y1;

    BB(Path2D.Double polygon) {
      boolean first = true;
      for (PathIterator pi = polygon.getPathIterator(null); pi.isDone() == false; ) {
        double[] coordinates = new double[6];
        int type = pi.currentSegment(coordinates);
        if (type == PathIterator.SEG_LINETO) {
          double x = coordinates[0];
          double y = coordinates[1];
          if (first) {
            x0 = x1 = x;
            y0 = y1 = y;
            first = false;
          } else {
            if (x < x0) {
              x0 = x;
            } else if (x1 < x) {
              x1 = x;
            }
            if (y < y0) {
              y0 = y;
            } else if (y1 < y) {
              y1 = y;
            }
          }
        }
        pi.next();
      }
    }

    // Can be put inside the polygon
    //BB(Polygon polygon) {
    //  boolean first = true;
    //  for (Double x: polygon.lons) {
    //    if (first) {
    //      x0 = x1 = x;
    //      first = false;
    //    } else {
    //      if (x < x0) {
    //        x0 = x;
    //      } else if (x1 < x) {
    //        x1 = x;
    //      }
    //    }
    //  }

    //  first = true;
    //  for (Double y: polygon.lats) {
    //    if (first) {
    //      y0 = y1 = y;
    //      first = false;
    //    } else {
    //      if (y < y0) {
    //        y0 = y;
    //      } else if (y1 < y) {
    //        y1 = y;
    //      }
    //    }
    //  }
    //}

    boolean contains(double x, double y) {
      return ((x0 <= x) && (x <= x1) && (y0 <= y) && (y <= y1));
    }
  }

  private static List<Path2D.Double> _polygons = new ArrayList();
  // Trying out a simple inclusion test
  //private static List<Polygon> _polygons = new ArrayList();

  private static List<BB> _BBs = new ArrayList();

  // Simple algorithm to test point in polygon
  //   https://stackoverflow.com/questions/8721406/how-to-determine-if-a-point-is-inside-a-2d-convex-polygon
  //
  // Java alrady have Polygon classes
  //   https://docs.oracle.com/javase/8/docs/api/java/awt/Polygon.html
  //	   This doesn't support Double coordinates
  //	 https://docs.oracle.com/javase/8/docs/api/java/awt/geom/Path2D.Double.html
  //	   This does
  private static void _Load() throws java.io.FileNotFoundException, java.io.IOException {
    String fn = String.format("%s/work/crawl-twitter/resource/us-states-map.txt", System.getProperty("user.home"));
    System.out.printf("Loading the USA map from %s ...\n", fn);

    try (BufferedReader br = new BufferedReader(new FileReader(fn))) {
      List<Double> lons = new ArrayList();
      List<Double> lats = new ArrayList();
      String line;
      while ((line = br.readLine()) != null) {
        // Right trim
        line = line.replaceAll("\\s+$","");

        if (line.length() == 0 || line.charAt(0) == '#') {
          // Finish a polygon
          if (lons.size() > 0) {
            // You need at least 3 points
            if (lons.size() < 3) {
              throw new RuntimeException(String.format("Unexpected: %d %d\n%s\n%s"
                    , lons.size()
                    , lats.size()
                    , org.apache.commons.lang3.StringUtils.join(lons, " ")
                    , org.apache.commons.lang3.StringUtils.join(lats, " ")
                    ));
            }

            Path2D.Double polygon = new Path2D.Double();
            for (int i = 0; i < lons.size(); i ++) {
              if (i == 0) {
                polygon.moveTo(lons.get(i), lats.get(i));
              }

              // We add all points including the last point.
              //   In the input file, some polygons have the same first and last points, others don't.
              polygon.lineTo(lons.get(i), lats.get(i));

              if (i == lons.size() - 1) {
                //double lon0 = lons.get(0);
                //double lon1 = lons.get(i);
                //double lat0 = lats.get(0);
                //double lat1 = lats.get(i);
                //if ( (lon1 != lon1) || (lat1 != lat0) )
                //	throw new RuntimeException(String.format("Unexpected:\n%s\n%s"
                //				, org.apache.commons.lang3.StringUtils.join(lons, " ")
                //				, org.apache.commons.lang3.StringUtils.join(lats, " ")
                //				));
                polygon.closePath();
              }
            }

            //Polygon polygon = new Polygon(lons, lats);

            lons.clear();
            lats.clear();

            _polygons.add(polygon);
            _BBs.add(new BB(polygon));
          }
          continue;
        }

        // The line should contains coordinates
        String t[] = line.split(" +");
        if (t.length != 2)
          throw new RuntimeException("Unexpected");
        double lat = Double.parseDouble(t[0]);
        double lon = Double.parseDouble(t[1]);
        lons.add(lon);
        lats.add(lat);
      }
    }

    System.out.printf("%d polygons loaded ...\n", _polygons.size());
  }

  static public boolean Contains(double lon, double lat) {
    // Optimizations:
    // - Quick reject by testing the point against the bounding box of the polygon.
    //   I'm guessing this will save time a lot. There are 574 polygons.
    // - Find the closest polygon from the point and start from there. You will need a R-tree for this.

    for (int i = 0; i < _polygons.size(); i ++) {
      if (_BBs.get(i).contains(lon, lat) && _polygons.get(i).contains(lon, lat))
        return true;

      //if (_polygons.get(i).contains(lon, lat))
      //  return true;
    }
    return false;
  }
}

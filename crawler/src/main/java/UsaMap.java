package crawltwitter;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.TimeUnit;
import java.util.List;
import java.awt.geom.Path2D;


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

      // 10,000 point test: 2010 ms.
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

  private static List<Path2D.Double> _polygons = new ArrayList();

  // Simple algorithm to test point in polygon
  //   https://stackoverflow.com/questions/8721406/how-to-determine-if-a-point-is-inside-a-2d-convex-polygon
  //
  // Java alrady have Polygon classes
  //   https://docs.oracle.com/javase/8/docs/api/java/awt/Polygon.html
  //	   This doesn't support Double coordinates
  //	 https://docs.oracle.com/javase/8/docs/api/java/awt/geom/Path2D.Double.html
  //	   This does
  private static void _Load() throws java.io.FileNotFoundException, java.io.IOException {
    String fn = String.format("%s/work/castnet/misc/us-states-map/usa.txt", System.getProperty("user.home"));
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

            lons.clear();
            lats.clear();

            _polygons.add(polygon);
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
    for (int i = 0; i < _polygons.size(); i ++) {
      if (_polygons.get(i).contains(lon, lat))
        return true;
    }
    return false;

    /*
    // Optimizations:
    // - Quick reject by testing the point against the bounding box of the polygon.
    //   I'm guessing this will save time a lot. There are 574 polygons.
    // - Find the closest polygon from the point and start from there. You will need a R-tree for this.

    // With the bounding-box quick-rejection optimization: 10 ms
    // Without: 129 ms
    auto pnt = gtl::construct<Point>(lon, lat);
    for (size_t i = 0; i < _BBs.size(); i ++) {
    if (_BBs[i].Contains(lon, lat) && gtl::contains(_polygons[i], pnt))
    return true;
    }

    return false;
    */
  }
}

/*
// http://www.boost.org/doc/libs/1_61_0/libs/polygon/doc/gtl_custom_polygon.htm

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

//first lets turn our polygon usage code into a generic
//function parameterized by polygon type
template <typename Polygon>
void test_polygon() {
//lets construct a 10x10 rectangle shaped polygon
typedef typename gtl::polygon_traits<Polygon>::point_type PPoint;

PPoint pts[] = {gtl::construct<PPoint>(0, 0),
gtl::construct<PPoint>(10, 0),
gtl::construct<PPoint>(10, 10),
gtl::construct<PPoint>(0, 10) };

Polygon poly;
gtl::set_points(poly, pts, pts+4);

cout << boost::format("area: %f\n") % gtl::area(poly);
cout << boost::format("point in polygon: %s\n") % gtl::contains(poly, gtl::construct<PPoint>(5, 5));
cout << boost::format("point in polygon: %s\n") % gtl::contains(poly, gtl::construct<PPoint>(15, 5));
}


//we have to get Point working with boost polygon to make our polygon
//that uses Point working with boost polygon
namespace boost { namespace polygon {
template <>
struct geometry_concept<Point> { typedef point_concept type; };
template <>
struct point_traits<Point> {
typedef double coordinate_type;

static inline coordinate_type get(const Point& point,
orientation_2d orient) {
if(orient == HORIZONTAL)
return point.x;
return point.y;
}
};

template <>
struct point_mutable_traits<Point> {
typedef double coordinate_type;

static inline void set(Point& point, orientation_2d orient, double value) {
if(orient == HORIZONTAL)
point.x = value;
else
point.y = value;
}
static inline Point construct(double x, double y) {
return Point(x, y);
}
};
} }

//I'm lazy and use the stl everywhere to avoid writing my own classes
//my toy polygon is a std::list<Point>
typedef std::list<Point> CPolygon;

//we need to specialize our polygon concept mapping in boost polygon
namespace boost { namespace polygon {
//first register CPolygon as a polygon_concept type
template <>
struct geometry_concept<CPolygon>{ typedef polygon_concept type; };

template <>
struct polygon_traits<CPolygon> {
  typedef double coordinate_type;
  typedef CPolygon::const_iterator iterator_type;
  typedef Point point_type;

  // Get the begin iterator
  static inline iterator_type begin_points(const CPolygon& t) {
    return t.begin();
  }

  // Get the end iterator
  static inline iterator_type end_points(const CPolygon& t) {
    return t.end();
  }

  // Get the number of sides of the polygon
  static inline std::size_t size(const CPolygon& t) {
    return t.size();
  }

  // Get the winding direction of the polygon
  static inline winding_direction winding(const CPolygon& t) {
    return unknown_winding;
  }
};

template <>
struct polygon_mutable_traits<CPolygon> {
  //expects stl style iterators
  template <typename iT>
    static inline CPolygon& set_points(CPolygon& t,
        iT input_begin, iT input_end) {
      t.clear();
      t.insert(t.end(), input_begin, input_end);
      return t;
    }

};
} }

namespace UsaMap {
  // Polygons of all states
  vector<CPolygon> _polygons;
  vector<BoundingBox> _BBs;


  bool Contains(double lon, double lat) {
    // Optimizations:
    // - Quick reject by testing the point against the bounding box of the polygon.
    //   I'm guessing this will save time a lot. There are 574 polygons.
    // - Not sure how this would help: finding the closest polygon from the
    //   point and start from there.

    // With the bounding-box quick-rejection optimization: 10 ms
    // Without: 129 ms
    if (true) {
      auto pnt = gtl::construct<Point>(lon, lat);
      for (size_t i = 0; i < _BBs.size(); i ++) {
        if (_BBs[i].Contains(lon, lat) && gtl::contains(_polygons[i], pnt))
          return true;
      }
    } else {
      auto pnt = gtl::construct<Point>(lon, lat);
      for (auto p: _polygons) {
        if (gtl::contains(p, pnt))
          return true;
      }
    }

    return false;
  }
  */

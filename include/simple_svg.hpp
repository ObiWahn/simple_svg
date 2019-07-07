
/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef SIMPLE_SVG_HPP
#define SIMPLE_SVG_HPP

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <iostream>
#include <optional>
#include <string>

namespace svg
{
    // Utility XML/String Functions.
    template <typename T>
    std::string attribute(std::string const & attribute_name,
        T const & value, std::string const & unit = "")
    {
        std::stringstream ss;
        ss << attribute_name << "=\"" << value << unit << "\" ";
        return ss.str();
    }
    std::string elemStart(std::string const & element_name)
    {
        return "\t<" + element_name + " ";
    }
    std::string elemEnd(std::string const & element_name)
    {
        return "</" + element_name + ">\n";
    }
    std::string emptyElemEnd()
    {
        return "/>\n";
    }

    // Quick optional return type.  This allows functions to return an invalid
    //  value if no good return is possible.  The user checks for validity
    //  before using the returned value.
    template <typename T>
    class optional
    {
    public:
        optional<T>(T const & type)
            : _valid(true), _type(type) { }
        optional<T>() : _valid(false), _type(T()) { }
        T * operator->()
        {
            // If we try to access an invalid value, an exception is thrown.
            if (!_valid)
                throw std::exception();

            return &_type;
        }
        // Test for validity.
        bool operator!() const { return !_valid; }
    private:
        bool _valid;
        T _type;
    };

    struct Dimensions
    {
        Dimensions(double width_, double height_) : width(width_), height(height_) { }
        Dimensions(double combined = 0) : width(combined), height(combined) { }
        double width;
        double height;
    };

    struct Point
    {
        Point(double x_ = 0, double y_ = 0) : x(x_), y(y_) { }
        double x;
        double y;
    };

    optional<Point> getMinPoint(std::vector<Point> const & points)
    {
        if (points.empty())
            return optional<Point>();

        Point min = points[0];
        for (unsigned i = 0; i < points.size(); ++i) {
            if (points[i].x < min.x)
                min.x = points[i].x;
            if (points[i].y < min.y)
                min.y = points[i].y;
        }
        return optional<Point>(min);
    }
    optional<Point> getMaxPoint(std::vector<Point> const & points)
    {
        if (points.empty())
            return optional<Point>();

        Point max = points[0];
        for (unsigned i = 0; i < points.size(); ++i) {
            if (points[i].x > max.x)
                max.x = points[i].x;
            if (points[i].y > max.y)
                max.y = points[i].y;
        }
        return optional<Point>(max);
    }

    // Defines the dimensions, scale, origin, and origin offset of the document.
    struct Layout
    {
        enum class Origin { TopLeft, BottomLeft, TopRight, BottomRight };

        Layout() = delete;
        explicit Layout(Dimensions const & _dimensions = Dimensions(400, 300)
              ,Dimensions const & _window = Dimensions(900, 900)
              ,Origin _origin = Origin::BottomLeft
              ,double _scale = 1
              ,Point const&  _origin_offset = Point(0,0))
            : dimensions(_dimensions)
            , window(_window)
            , scale(_scale)
            , origin(_origin)
            , origin_offset(_origin_offset)
            { }
        Dimensions dimensions;
        Dimensions window;
        double scale;
        Origin origin;
        Point origin_offset;
    };

    // Convert coordinates in user space to SVG native space.
    double translateX(Layout const & layout, double x, double w = 0 )
    {
        auto x_out = (x + layout.origin_offset.x) * layout.scale;
        switch (layout.origin) {
            case Layout::Origin::TopLeft:
            case Layout::Origin::BottomLeft:
                return x_out;
            case Layout::Origin::TopRight:
            case Layout::Origin::BottomRight:
            default: //avoid warnging
                return layout.dimensions.width - x_out - w;
        }
    }

    double translateY(Layout const & layout, double y, double h = 0 )
    {
        auto y_out = (y + layout.origin_offset.y) * layout.scale;

        switch (layout.origin) {
            case Layout::Origin::TopLeft:
            case Layout::Origin::TopRight:
                return y_out;
            case Layout::Origin::BottomLeft:
            case Layout::Origin::BottomRight:
            default: //avoid warnging
                return layout.dimensions.height - y_out - h;
        }
    }

    double translateScale(double dimension, Layout const & layout)
    {
        return dimension * layout.scale;
    }

    class Serializeable
    {
    public:
        Serializeable() { }
        virtual ~Serializeable() { };
        virtual std::string toString(Layout const & layout) const = 0;
    };

    class Color : public Serializeable
    {
    public:
        enum Defaults { Transparent = -1, Aqua, Black, Blue, Brown, Cyan, Fuchsia,
            Green, Lime, Magenta, Orange, Purple, Red, Silver, White, Yellow };

        Color(int r, int g, int b) : transparent(false), red(r), green(g), blue(b) { }
        Color(Defaults color)
            : transparent(false), red(0), green(0), blue(0)
        {
            switch (color)
            {
                case Aqua: assign(0, 255, 255); break;
                case Black: assign(0, 0, 0); break;
                case Blue: assign(0, 0, 255); break;
                case Brown: assign(165, 42, 42); break;
                case Cyan: assign(0, 255, 255); break;
                case Fuchsia: assign(255, 0, 255); break;
                case Green: assign(0, 128, 0); break;
                case Lime: assign(0, 255, 0); break;
                case Magenta: assign(255, 0, 255); break;
                case Orange: assign(255, 165, 0); break;
                case Purple: assign(128, 0, 128); break;
                case Red: assign(255, 0, 0); break;
                case Silver: assign(192, 192, 192); break;
                case White: assign(255, 255, 255); break;
                case Yellow: assign(255, 255, 0); break;
                default: transparent = true; break;
            }
        }
        virtual ~Color() { }
        std::string toString(Layout const &) const
        {
            std::stringstream ss;
            if (transparent)
                ss << "transparent";
            else
                ss << "rgb(" << red << "," << green << "," << blue << ")";
            return ss.str();
        }
    private:
            bool transparent;
            int red;
            int green;
            int blue;

            void assign(int r, int g, int b)
            {
                red = r;
                green = g;
                blue = b;
            }
    };

    class Fill : public Serializeable
    {
    public:
        Fill(Color::Defaults color_) : color(color_) { }
        Fill(Color color_ = Color::Transparent)
            : color(color_) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << attribute("fill", color.toString(layout));
            return ss.str();
        }
    private:
        Color color;
    };

    class Stroke : public Serializeable
    {
    public:
        Stroke(double width_ = -1, Color color_ = Color::Transparent)
            : width(width_), color(color_) { }
        std::string toString(Layout const & layout) const
        {
            // If stroke width is invalid.
            if (width < 0)
                return std::string();

            std::stringstream ss;
            ss << attribute("stroke-width", translateScale(width, layout)) << attribute("stroke", color.toString(layout));
            return ss.str();
        }
    private:
        double width;
        Color color;
    };

    class Font : public Serializeable
    {
    public:
        Font(double size_ = 12, std::string const & family_ = "Verdana") : size(size_), family(family_) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << attribute("font-size", translateScale(size, layout)) << attribute("font-family", family);
            return ss.str();
        }
    private:
        double size;
        std::string family;
    };

    class Shape : public Serializeable
    {
    public:
        Shape(Fill const & fill_ = Fill(), Stroke const & stroke_ = Stroke())
            : fill(fill_), stroke(stroke_) { }
        virtual ~Shape() { }
        virtual std::string toString(Layout const & layout) const = 0;
        virtual void offset(Point const & offset) = 0;
    protected:
        Fill fill;
        Stroke stroke;
    };

    template <typename T>
    std::string vectorToString(std::vector<T> collection, Layout const & layout)
    {
        std::string combination_str;
        for (unsigned i = 0; i < collection.size(); ++i)
            combination_str += collection[i].toString(layout);

        return combination_str;
    }

    class Circle : public Shape
    {
    public:
        Circle(Point const & center_, double diameter_, Fill const & fill_,
            Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_), center(center_), radius(diameter_ / 2) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("circle") << attribute("cx", translateX(layout, center.x))
                << attribute("cy", translateY(layout, center.y))
                << attribute("r", translateScale(radius, layout)) << fill.toString(layout)
                << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset)
        {
            center.x += offset.x;
            center.y += offset.y;
        }
    private:
        Point center;
        double radius;
    };

    class Elipse : public Shape
    {
    public:
        Elipse(Point const & center_, double width_, double height_,
            Fill const & fill_ = Fill(), Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_), center(center_), radius_width(width_ / 2),
            radius_height(height_ / 2) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("ellipse") << attribute("cx", translateX(layout, center.x))
                << attribute("cy", translateY(layout, center.y))
                << attribute("rx", translateScale(radius_width, layout))
                << attribute("ry", translateScale(radius_height, layout))
                << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset)
        {
            center.x += offset.x;
            center.y += offset.y;
        }
    private:
        Point center;
        double radius_width;
        double radius_height;
    };

    class Rectangle : public Shape
    {
    public:
        Rectangle(Point const & edge_, double width_, double height_,
            Fill const & fill_ = Fill(), Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_), edge(edge_), width(width_),
            height(height_) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("rect")
               << attribute("x", translateX(layout, edge.x, width))
               << attribute("y", translateY(layout, edge.y, height))
               << attribute("width", translateScale(width, layout))
               << attribute("height", translateScale(height, layout))
               << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset)
        {
            edge.x += offset.x;
            edge.y += offset.y;
        }
    private:
        Point edge; // vector to origin (top left point)
        double width;
        double height;
    };

    class Line : public Shape
    {
    public:
        Line(Point const & start_point_, Point const & end_point_,
            Stroke const & stroke_ = Stroke())
            : Shape(Fill(), stroke_), start_point(start_point_),
            end_point(end_point_) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("line") << attribute("x1", translateX(layout, start_point.x))
                << attribute("y1", translateY(layout, start_point.y))
                << attribute("x2", translateX(layout, end_point.x))
                << attribute("y2", translateY(layout, end_point.y))
                << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset)
        {
            start_point.x += offset.x;
            start_point.y += offset.y;

            end_point.x += offset.x;
            end_point.y += offset.y;
        }
    private:
        Point start_point;
        Point end_point;
    };

    class Polygon : public Shape
    {
    public:
        Polygon(Fill const & fill_ = Fill(), Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_) { }
        Polygon(Stroke const & stroke_ = Stroke()) : Shape(Color::Transparent, stroke_) { }
        Polygon & operator<<(Point const & point)
        {
            points.push_back(point);
            return *this;
        }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("polygon");

            ss << "points=\"";
            for (unsigned i = 0; i < points.size(); ++i)
                ss << translateX(layout, points[i].x) << "," << translateY(layout, points[i].y) << " ";
            ss << "\" ";

            ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset)
        {
            for (unsigned i = 0; i < points.size(); ++i) {
                points[i].x += offset.x;
                points[i].y += offset.y;
            }
        }
    private:
        std::vector<Point> points;
    };

    class Polyline : public Shape
    {
    public:
        Polyline(Fill const & fill_ = Fill(), Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_) { }
        Polyline(Stroke const & stroke_ = Stroke()) : Shape(Color::Transparent, stroke_) { }
        Polyline(std::vector<Point> const & points_,
            Fill const & fill_ = Fill(), Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_), points(points_) { }
        Polyline & operator<<(Point const & point)
        {
            points.push_back(point);
            return *this;
        }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("polyline");

            ss << "points=\"";
            for (unsigned i = 0; i < points.size(); ++i)
                ss << translateX(layout, points[i].x) << "," << translateY(layout, points[i].y) << " ";
            ss << "\" ";

            ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
            return ss.str();
        }
        void offset(Point const & offset)
        {
            for (unsigned i = 0; i < points.size(); ++i) {
                points[i].x += offset.x;
                points[i].y += offset.y;
            }
        }
        std::vector<Point> points;
    };

    class Text : public Shape
    {
    public:
        Text(Point const & origin_, std::string const & content_, Fill const & fill_ = Fill(),
             Font const & font_ = Font(), Stroke const & stroke_ = Stroke())
            : Shape(fill_, stroke_), origin(origin_), content(content_), font(font_) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << elemStart("text") << attribute("x", translateX(layout, origin.x))
                << attribute("y", translateY(layout, origin.y))
                << fill.toString(layout) << stroke.toString(layout) << font.toString(layout)
                << ">" << content << elemEnd("text");
            return ss.str();
        }
        void offset(Point const & offset)
        {
            origin.x += offset.x;
            origin.y += offset.y;
        }
    private:
        Point origin;
        std::string content;
        Font font;
    };

    // Sample charting class.
    class LineChart : public Shape
    {
    public:
        LineChart(Dimensions margin_ = Dimensions(), double scale_ = 1,
                  Stroke const & axis_stroke_ = Stroke(.5, Color::Purple))
            : axis_stroke(axis_stroke_), margin(margin_), scale(scale_) { }
        LineChart & operator<<(Polyline const & polyline)
        {
            if (polyline.points.empty())
                return *this;

            polylines.push_back(polyline);
            return *this;
        }
        std::string toString(Layout const & layout) const
        {
            if (polylines.empty())
                return "";

            std::string ret;
            for (unsigned i = 0; i < polylines.size(); ++i)
                ret += polylineToString(polylines[i], layout);

            return ret + axisString(layout);
        }
        void offset(Point const & offset)
        {
            for (unsigned i = 0; i < polylines.size(); ++i)
                polylines[i].offset(offset);
        }
    private:
        Stroke axis_stroke;
        Dimensions margin;
        double scale;
        std::vector<Polyline> polylines;

        optional<Dimensions> getDimensions() const
        {
            if (polylines.empty())
                return optional<Dimensions>();

            optional<Point> min = getMinPoint(polylines[0].points);
            optional<Point> max = getMaxPoint(polylines[0].points);
            for (unsigned i = 0; i < polylines.size(); ++i) {
                if (getMinPoint(polylines[i].points)->x < min->x)
                    min->x = getMinPoint(polylines[i].points)->x;
                if (getMinPoint(polylines[i].points)->y < min->y)
                    min->y = getMinPoint(polylines[i].points)->y;
                if (getMaxPoint(polylines[i].points)->x > max->x)
                    max->x = getMaxPoint(polylines[i].points)->x;
                if (getMaxPoint(polylines[i].points)->y > max->y)
                    max->y = getMaxPoint(polylines[i].points)->y;
            }

            return optional<Dimensions>(Dimensions(max->x - min->x, max->y - min->y));
        }
        std::string axisString(Layout const & layout) const
        {
            optional<Dimensions> dimensions = getDimensions();
            if (!dimensions)
                return "";

            // Make the axis 10% wider and higher than the data points.
            double width = dimensions->width * 1.1;
            double height = dimensions->height * 1.1;

            // Draw the axis.
            Polyline axis(Color::Transparent, axis_stroke);
            axis << Point(margin.width, margin.height + height) << Point(margin.width, margin.height)
                << Point(margin.width + width, margin.height);

            return axis.toString(layout);
        }
        std::string polylineToString(Polyline const & polyline, Layout const & layout) const
        {
            Polyline shifted_polyline = polyline;
            shifted_polyline.offset(Point(margin.width, margin.height));

            std::vector<Circle> vertices;
            for (unsigned i = 0; i < shifted_polyline.points.size(); ++i)
                vertices.push_back(Circle(shifted_polyline.points[i], getDimensions()->height / 30.0, Color::Black));

            return shifted_polyline.toString(layout) + vectorToString(vertices, layout);
        }
    };

    class Document
    {
    public:
        explicit Document(std::string const & file_name, Layout layout_)
            : layout(layout_)
            , stream_real(file_name)
            , stream(stream_real.value())
            { }

        explicit Document(std::ostream& out, Layout layout_)
            : layout(layout_)
            , stream(out)
            { }

        Document & operator<<(Shape const & shape)
        {
            body_nodes_str += shape.toString(layout);
            return *this;
        }
        std::string toString() const
        {
            using namespace std::literals::string_literals;
            std::stringstream ss;
            ss << "<?xml "
               << attribute("version", "1.0")
               << attribute("standalone", "no")
               << "?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
               << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg "
               << attribute("width", layout.window.width, "px")
               << attribute("height", layout.window.height, "px")
               << attribute("viewBox", "0 0 "s + std::to_string(layout.dimensions.width) +" "s + std::to_string(layout.dimensions.height) )
               << attribute("preserveAspectRatio", "xMinYMin meet")
               << attribute("xmlns", "http://www.w3.org/2000/svg")
               << attribute("version", "1.1") << ">\n"
               << body_nodes_str << elemEnd("svg");
            return ss.str();
        }
        bool save()
        {
            if (!stream.good())
                return false;

            stream << toString();
            if (stream_real){
                stream_real.value().close();
            }
            return true;
        }
    private:
        Layout layout;
        std::optional<std::ofstream> stream_real;
        std::ostream& stream;

        std::string body_nodes_str;
    };
}

#endif

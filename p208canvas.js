/*
   Copyright (c) 2014 Samuel Rødal

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

// solution visualizer for Project Euler problem 208

(function() {
    var segments = 30;

    function init(canvas, width, height) {
        var ctx = canvas.getContext('2d');

        var constant = function(x) {
            return function() { return x; };
        };

        function append(head, tail) {
            return [head, constant(tail)];
        };

        function head(lst) {
            return lst[0];
        };

        function tail(lst) {
            return lst[1]();
        };

        function map(f, lst) {
            return lst &&
                [f(head(lst)),
                 function() { return map(f, tail(lst)); }];
        };

        function consume(f, lst) {
            return lst && [f(head(lst)), consume(f, tail(lst))];
        };

        function chain(lst, thunk) {
            return lst && [head(lst),
                           function() {
                               return chain(tail(lst), thunk)
                           }]
                   || thunk && thunk();
        };

        function curry(f) {
            var curried = Array.prototype.slice.call(arguments, 1);
            return function() {
                var args = curried.concat(Array.prototype.slice.call(arguments));
                return f.apply(null, args);
            };
        };

        function compose(f, g) {
            return function() {
                var args = Array.prototype.slice.call(arguments);
                return f(g.apply(null, args));
            };
        };

        function memoize(f) {
            var cache = [];

            var lookup = function() {
                var args = Array.prototype.slice.call(arguments);

                var s = this[args[0]];
                if (!s || args.length == 1)
                    return s;
                else
                    return lookup.apply(s, args.slice(1));
            };

            var save = function() {
                var args = Array.prototype.slice.call(arguments);

                if (args.length == 2) {
                    this[args[0]] = args[1];
                } else {
                    this[args[0]] = this[args[0]] || [];
                    save.apply(this[args[0]], args.slice(1));
                }
            };

            return function() {
                var args = Array.prototype.slice.call(arguments);

                var result = lookup.apply(cache, args);
                if (!result) {
                    var result = f.apply(null, args);

                    args.push(result);
                    save.apply(cache, args);
                }

                return result;
            };
        };

        var solve = memoize(function(n1, n2, n3, n4, n5) {
            if (n1 + n2 + n3 + n4 + n5 == 0)
                return append(null, null);

            var append1 = curry(append, 1);
            var negate = function(x) {
                return -x;
            };

            var left = function () {
                return map(compose(curry(map, negate), append1),
                           solve(n1-1, n5, n4, n3, n2));
            };

            var right = function () {
                return map(append1, solve(n2-1, n3, n4, n5, n1));
            };

            return chain(n1 > 0 && left(), n2 > 0 && right);
        });

        var n = segments / 5;
        var paths = solve(n, n, n, n, n);

        var count = 1;

        var nextPath = function() {
            if (!paths)
                return;

            var path = head(paths);

            ctx.save();

            ctx.fillStyle = "#d4e2d6";
            ctx.fillRect(0, 0, width, height);

            ctx.translate(width/2, height/2);

            ctx.rotate(-90*Math.PI/180);
            ctx.scale(0.7, 0.7);

            var i = 0

            var r = 15;
            var e = 2*r*Math.sin(Math.PI/5);
            var h = Math.sqrt(r*r - (e/2)*(e/2))

            first = prev = head(path);
            f = function(rotation) {
                ctx.strokeStyle =
                    ['rgba(200,50,40, 0.7)','rgba(40,30,110,0.7)'][i++ % 2];

                ctx.beginPath();
                if (prev > 0) {
                    ctx.arc(e/2, h, r,
                            (270-72/2)*Math.PI/180,
                            (270+72/2)*Math.PI/180, false);
                } else {
                    ctx.arc(e/2, -h, r,
                            (90-72/2)*Math.PI/180,
                            (90+72/2)*Math.PI/180, false);
                }
                ctx.lineWidth = 10;
                ctx.opacity = 0.5
                ctx.stroke();
                ctx.translate(e,0);
                if (rotation == prev)
                    ctx.rotate(72*rotation*Math.PI/180);

                prev = rotation;
            };

            consume(f, chain(tail(path), constant(append(first, null))));

            ctx.restore();

            paths = tail(paths);
        };

        var animate = function(animation, interval) {
            interval = interval || 1;
            var i = interval;
            var running = true;

            var anim = function()
            {
                if (--i == 0) {
                    animation();
                    i = interval;
                }

                if (running)
                    requestAnimationFrame(anim);
            };

            anim();

            return function() {
                if (running = !running)
                    anim();
            };
        };

        var toggle = animate(nextPath, 2);

        canvas.onmouseup = function(e) {
            toggle();
            canvas.isDrawing = false;
        };
    };

    var width = 512;
    var height = 512;

    canvas = document.createElement('canvas');
    canvas.width = width;
    canvas.height = height;

    document.body.appendChild(canvas);

    init(canvas, width, height);
})();


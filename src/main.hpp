//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_MAIN_HPP
#define ZENGINE_MAIN_HPP

#include "application.hpp"

extern application * g_app;


class z_main
        {
    application app;

        public:
            void make_current() { g_app = &this->app; }

            z_main() noexcept { make_current(); }

            z_main(int argc, const char * const argv[]);

            void run();

            ~z_main();

};


#endif //ZENGINE_MAIN_HPP

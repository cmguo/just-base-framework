// Param.h

#ifndef _FRAMEWORK_LOGGER_PARAM_H_
#define _FRAMEWORK_LOGGER_PARAM_H_

namespace framework
{
    namespace logger
    {

        template <
            typename _Ty
        >
        struct ParamRef
        {
            ParamRef(
                _Ty const & value)
                : value_(value)
            {
            }

            _Ty const & value() const
            {
                return value_;
            }

        private:
            _Ty const & value_;
        };

        template <
            typename _Ty, 
            size_t size
        >
        struct ParamRef<_Ty [size]>
        {
            ParamRef(
                _Ty const (& value)[size])
                : value_(value)
            {
            }

            _Ty const * value() const
            {
                return value_;
            }

        private:
            _Ty const * value_;
        };

        template <
            typename _Ty
        >
        class Param
            : ParamRef<_Ty>
        {
        public:
            Param(
                _Ty const & value)
                : ParamRef<_Ty>(value)
            {
            }

        public:
            template <
                typename Formator
            >
            void format(
                Formator & fmt) const
            {
                fmt(this->value());
            }
        };

        class NullParam
        {
        public:
            template <
                typename Formator
            >
            void format(
                Formator & fmt) const
            {
            }
        };

        template <
            typename _Front, 
            typename _Back
        >
        class ParamPair
        {
        public:
            ParamPair()
            {
            }

            ParamPair(
                _Front const & front, 
                _Back const & back)
                : front_(front)
                , back_(back)
            {
            }

        public:
            typedef ParamPair this_type;
            typedef _Front front_type;
            typedef _Back back_type;

            template <
                typename Back
            >
            ParamPair<this_type, Param<Back> > const operator()(
                Back const & back) const
            {
                return ParamPair<this_type, Param<Back> >(*this, back);
            }

            template <
                typename Back
            >
            ParamPair<this_type, Param<Back> > const operator%(
                Back const & back) const
            {
                return (*this)(back);
            }

            template <
                typename Back
            >
            ParamPair<this_type, Param<Back> > const operator<<(
                Back const & back) const
            {
                return (*this)(back);
            }

        public:
            template <
                typename Formator
            >
            void format(
                Formator & fmt) const
            {
                front_.format(fmt);
                back_.format(fmt);
            }

        private:
            front_type front_;
            back_type back_;
        };

        typedef ParamPair<NullParam, NullParam> ParamsBegin;

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_PARAM_H_

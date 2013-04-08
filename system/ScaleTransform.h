// ScaleTransform.h

#ifndef _FRAMEWORK_SYSTEM_SCALE_TRANSFORM_H_
#define _FRAMEWORK_SYSTEM_SCALE_TRANSFORM_H_

namespace framework
{
    namespace system
    {

        /* 
            at the beginning
                out * scale_in + left = in * scale_out
            let in = 1, then
                out * scale_in + left = scale_out
            because left < scale_in, so we have
                out = scale_out / scale_in
                left = scale_out - out * scale_in
         */

        class ScaleTransform
        {
        public:
            ScaleTransform()
                : scale_in_(1)
                , scale_out_(1)
                , last_in_(0)
                , last_out_(0)
                , last_left_(0)
            {
            }

            ScaleTransform(
                boost::uint64_t scale_in, 
                boost::uint64_t scale_out, 
                boost::uint64_t init_out = 0, 
                boost::uint64_t init_in = 0)
                : scale_in_(1)
                , scale_out_(1)
                , last_in_(init_in)
                , last_out_(init_out)
                , last_left_(0)
            {
                calc_scale(scale_in, scale_out);
            }

        public:
            static boost::uint64_t static_transfer(
                boost::uint64_t scale_in, 
                boost::uint64_t scale_out, 
                boost::uint64_t n, 
                boost::uint64_t * l = NULL)
            {
                boost::uint64_t out = scale_out / scale_in;
                boost::uint64_t left = scale_out - out * scale_in;
                boost::uint64_t out2 = 0;
                boost::uint64_t left2 = 0;
                while (n) {
                    if (n & 1) {
                        out2 += out;
                        left2 += left;
                        if (left2 >= scale_in) {
                            ++out2;
                            left2 -= scale_in;
                        }
                    }
                    n >>= 1;
                    out <<= 1;
                    left <<= 1;
                    if (left >= scale_in) {
                        ++out;
                        left -= scale_in;
                    }
                }
                if (l) {
                    *l = left2;
                }
                return out2;
            }

            // 与直接反向转换不同的是，它保留了转换双方之间的偏移量
            boost::uint64_t revert(
                boost::uint64_t n, 
                boost::uint64_t * l = NULL) const
            {
                if (scale_in_ == 1) {
                    if (scale_out_ == 1) {
                        return n;
                    } else {
                        return n / scale_out_;
                    }
                }
                boost::uint64_t scale_in = scale_out_;
                boost::uint64_t scale_out = scale_in_;
                boost::uint64_t in = last_out_;
                boost::uint64_t out = last_in_;
                boost::uint64_t left = last_left_;
                if (left) {
                    --out;
                    left = scale_in - left;
                }
                boost::uint64_t left2 = 0;
                if ((boost::int64_t)n > (boost::int64_t)in) {
                    out += static_transfer(scale_in, scale_out, n - in, &left2);
                    left += left2;
                    if (left >= scale_in) {
                        ++out;
                        left -= scale_in;
                    }
                } else if ((boost::int64_t)n < (boost::int64_t)in) {
                    out -= static_transfer(scale_in, scale_out, in - n, &left2);
                    if (left < left2) {
                        --out;
                        left = scale_in + left - left2;
                    } else {
                        left -= left2;
                    }
                }
                if (l) {
                    *l = left * common_divisor_;
                }
                return out;
            }

        public:
            void reset(
                boost::uint64_t out = 0, 
                boost::uint64_t in = 0)
            {
                last_in_ = in;
                last_out_ = out;
                last_left_ = 0;
            }

            void reset_scale(
                boost::uint64_t scale_in, 
                boost::uint64_t scale_out, 
                boost::uint64_t init_out = 0, 
                boost::uint64_t init_in = 0)
            {
                calc_scale(scale_in, scale_out);
                last_in_ = init_in;
                last_out_ = init_out;
                last_left_ = 0;
            }

        public:
            boost::uint64_t inc()
            {
                ++last_in_;
                last_out_ += trans1_[1].out;
                last_left_ += trans1_[1].left;
                if (last_left_ >= scale_in_) {
                    ++last_out_;
                    last_left_ -= scale_in_;
                }
                return last_out_;
            }

            boost::uint64_t inc(
                boost::uint64_t n)
            {
                last_in_ += n;
                if (scale_in_ == 1) {
                    if (scale_out_ == 1) {
                        return last_out_ += n;
                    } else {
                        last_out_ += n * scale_out_;
                        return last_out_;
                    }
                }
                last_out_ += trans1_[n & 0xff].out;
                last_left_ += trans1_[n & 0xff].left;
                if (last_left_ >= scale_in_) {
                    ++last_out_;
                    last_left_ -= scale_in_;
                }
                n >>= 8;
                if (n) {
                    size_t i = 0;
                    do {
                        if (n & 1) {
                            last_out_ += trans2_[i].out;
                            last_left_ += trans2_[i].left;
                            if (last_left_ >= scale_in_) {
                                ++last_out_;
                                last_left_ -= scale_in_;
                            }
                        }
                        n >>= 1;
                        ++i;
                    } while (n);
                }
                return last_out_;
            }

            boost::uint64_t dec(
                boost::uint64_t n)
            {
                last_in_ -= n;
                if (scale_in_ == 1) {
                    if (scale_out_ == 1) {
                        return last_out_ -= n;
                    } else {
                        last_out_ -= n * scale_out_;
                        return last_out_;
                    }
                }
                last_out_ -= trans1_[n & 0xff].out;
                if (last_left_ >= trans1_[n & 0xff].left) {
                    last_left_ -= trans1_[n & 0xff].left;
                } else {
                    --last_out_;
                    last_left_ += scale_in_ - trans1_[n & 0xff].left;
                }
                n >>= 8;
                if (n) {
                    size_t i = 0;
                    do {
                        if (n & 1) {
                            last_out_ -= trans2_[i].out;
                            if (last_left_ >= trans2_[i].left) {
                                last_left_ -= trans2_[i].left;
                            } else {
                                --last_out_;
                                last_left_ += scale_in_ - trans2_[i].left;
                            }
                        }
                        n >>= 1;
                        ++i;
                    } while (n);
                }
                return last_out_;
            }

            boost::uint64_t transfer(
                boost::uint64_t n)
            {
                return ((boost::int64_t)n > (boost::int64_t)last_in_) 
                    ? inc(n - last_in_) 
                    : dec(last_in_ - n);
            }

        public:
            // 如果不希望记录本次转换的输入输出，则使用下列 const 版本接口

            boost::uint64_t inc(
                boost::uint64_t * l = NULL) const
            {
                boost::uint64_t out = last_out_ + trans1_[1].out;
                boost::uint64_t left = last_left_ + trans1_[1].left;
                if (left >= scale_in_) {
                    ++out;
                }
                if (l) {
                    *l = left;
                }
                return out;
            }

            boost::uint64_t inc(
                boost::uint64_t n, 
                boost::uint64_t * l = NULL) const
            {
                if (scale_in_ == 1) {
                    if (scale_out_ == 1) {
                        return last_out_ + n;
                    } else {
                        return last_out_ + n * scale_out_;
                    }
                }
                boost::uint64_t out = last_out_ + trans1_[n & 0xff].out;
                boost::uint64_t left = last_left_ + trans1_[n & 0xff].left;
                if (left >= scale_in_) {
                    ++out;
                    left -= scale_in_;
                }
                n >>= 8;
                if (n) {
                    size_t i = 0;
                    do {
                        if (n & 1) {
                            out += trans2_[i].out;
                            left += trans2_[i].left;
                            if (left >= scale_in_) {
                                ++out;
                                left -= scale_in_;
                            }
                        }
                        n >>= 1;
                        ++i;
                    } while (n);
                }
                if (l) {
                    *l = left;
                }
                return out;
            }

            boost::uint64_t dec(
                boost::uint64_t n, 
                boost::uint64_t * l = NULL) const
            {
                if (scale_in_ == 1) {
                    if (scale_out_ == 1) {
                        return last_out_ - n;
                    } else {
                        return last_out_ - n * scale_out_;
                    }
                }
                boost::uint64_t out = last_out_ - trans1_[n & 0xff].out;
                boost::uint64_t left = last_left_;
                if (left >= trans1_[n & 0xff].left) {
                    left -= trans1_[n & 0xff].left;
                } else {
                    --out;
                    left += scale_in_ - trans1_[n & 0xff].left;
                }
                n >>= 8;
                if (n) {
                    size_t i = 0;
                    do {
                        if (n & 1) {
                            out -= trans2_[i].out;
                            if (left >= trans2_[i].left) {
                                left -= trans2_[i].left;
                            } else {
                                --out;
                                left += scale_in_ - trans2_[i].left;
                            }
                        }
                        n >>= 1;
                        ++i;
                    } while (n);
                }
                if (l) {
                    *l = left;
                }
                return out;
            }

            boost::uint64_t transfer(
                boost::uint64_t n, 
                boost::uint64_t * l = NULL) const
            {
                return ((boost::int64_t)n > (boost::int64_t)last_in_) 
                    ? inc(n - last_in_, l) 
                    : dec(last_in_ - n, l);
            }

        public:
            // 下面两个接口单独设置 in 或者 out，实际上是要反映转换双方之间的偏移量

            void last_in(
                boost::uint64_t in)
            {
                last_in_ = in;
            }

            void last_out(
                boost::uint64_t out, 
                boost::uint64_t left = 0)
            {
                last_out_ = out;
                last_left_ = left;
            }

        public:
            boost::uint64_t get() const
            {
                return last_out_;
            }

            boost::uint64_t last_in() const
            {
                return last_in_;
            }

            boost::uint64_t last_out(
                boost::uint64_t * left = NULL) const
            {
                if (left) {
                    *left = last_left_ * common_divisor_;
                }
                return scale_out_;
            }

            boost::uint64_t scale_in() const
            {
                return scale_in_ * common_divisor_;
            }

            boost::uint64_t scale_out() const
            {
                return scale_out_ * common_divisor_;
            }

        private:
            void calc_scale(
                boost::uint64_t scale_in, 
                boost::uint64_t scale_out)
            {
                common_divisor_ = common_divisor(scale_in, scale_out);
                scale_in_ = scale_in / common_divisor_;
                scale_out_ = scale_out / common_divisor_;
                Transform t = {0, 0};
                trans1_.resize(256);
                for (size_t i = 0; i < 256; ++i) {
                    trans1_[i] = t;
                    t.left += scale_out_;
                    if (t.left >= scale_in_) {
                        boost::uint64_t n = t.left / scale_in_;
                        t.out += n;
                        t.left -= n * scale_in_;
                    }
                }
                trans2_.resize(56);
                for (size_t i = 0; i < 56; ++i) {
                    trans2_[i] = t;
                    t.out <<= 1;
                    t.left <<= 1;
                    if (t.left >= scale_in_) {
                        ++t.out;
                        t.left -= scale_in_;
                    }
                }
            }

            static boost::uint64_t common_divisor(
                boost::uint64_t scale_in, 
                boost::uint64_t scale_out)
            {
                while (scale_in != scale_out) {
                    if (scale_in > scale_out) {
                        scale_in -= scale_in / scale_out * scale_out;
                        if (scale_in == 0)
                            return scale_out;
                    } else {
                        scale_out -= scale_out / scale_in * scale_in;
                        if (scale_out == 0)
                            return scale_in;
                    }
                }
                return scale_in;
            }

        private:
            struct Transform
            {
                boost::uint64_t out;
                boost::uint64_t left;
            };

            boost::uint64_t scale_in_;
            boost::uint64_t scale_out_;
            boost::uint64_t common_divisor_;
            std::vector<Transform> trans1_;
            std::vector<Transform> trans2_;

            boost::uint64_t last_in_;
            boost::uint64_t last_out_;
            boost::uint64_t last_left_;
        };

    }
}

#endif // _FRAMEWORK_SYSTEM_SCALE_TRANSFORM_H_


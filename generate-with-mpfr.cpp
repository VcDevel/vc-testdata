#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <string>
#define MPFR_WANT_FLOAT128
#include <mpfr.h>

template <typename MathFun> class Fun1
{
  using FilePtr = std::FILE*;
  const char* fun_name;
  FilePtr qpfile, epfile, dpfile, spfile;
  mpfr_t qx, q, tmp;
  mpfr_t ex, e;
  mpfr_t dx, d;
  mpfr_t sx, s;
  MathFun math_fun;

  void print () {
    mpfr_set(ex, qx, MPFR_RNDN);
    mpfr_set(dx, qx, MPFR_RNDN);
    mpfr_set(sx, qx, MPFR_RNDN);

    math_fun(q, qx, MPFR_RNDN);
    math_fun(e, ex, MPFR_RNDN);
    math_fun(d, dx, MPFR_RNDN);
    math_fun(s, sx, MPFR_RNDN);

    for (auto toprint : {qx, q}) {
      const __float128 tmp = mpfr_get_float128(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, qpfile);
    }
    for (auto toprint : {ex, e}) {
      const long double tmp = mpfr_get_ld(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, epfile);
    }
    for (auto toprint : {dx, d}) {
      const double tmp = mpfr_get_d(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, dpfile);
    }
    for (auto toprint : {sx, s}) {
      const float tmp = mpfr_get_flt(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, spfile);
    }
  };

public:
  Fun1(const char* function_name, MathFun _math_fun)
      : fun_name(function_name), math_fun(_math_fun)
  {
    std::string filename("reference-");
    filename.append(function_name);
    qpfile = std::fopen((filename + "-qp.dat").c_str(), "w");
    epfile = std::fopen((filename + "-ep.dat").c_str(), "w");
    dpfile = std::fopen((filename + "-dp.dat").c_str(), "w");
    spfile = std::fopen((filename + "-sp.dat").c_str(), "w");

    mpfr_inits2(113, qx, q, tmp, mpfr_ptr(nullptr));
    mpfr_inits2(64, ex, e, mpfr_ptr(nullptr));
    mpfr_inits2(53, dx, d, mpfr_ptr(nullptr));
    mpfr_inits2(24, sx, s, mpfr_ptr(nullptr));
  }

  ~Fun1() {
    std::fclose(qpfile);
    std::fclose(epfile);
    std::fclose(dpfile);
    std::fclose(spfile);
    mpfr_clears(qx, q, ex, e, dx, d, sx, s, mpfr_ptr(nullptr));
  }

  template <typename F> Fun1& loop(long n, const char* range, F&& init_fun)
  {
    std::printf("\nGenerating %d qp, ep, dp, and sp %s values %s     ", int(n), fun_name,
                range);
    for (long i = 0; i < n; ++i) {
      init_fun(i, qx);
      print();
      std::printf("\033[4D%3i%%", int(i * 100 / (n - 1)));
    }
    return *this;
  }
};

constexpr long n = 32 * 97 * 89;
constexpr long n2 = n / 11;

template <long From = 0, long To = 1> void init(long i, mpfr_t& x)
{
  mpfr_set_si(x, i, MPFR_RNDN);
  mpfr_mul_si(x, x, To - From, MPFR_RNDN);
  mpfr_div_si(x, x, n, MPFR_RNDN);
  mpfr_add_si(x, x, From, MPFR_RNDN);
};

template <long From = 0, long To = 1> void init_pi4(long i, mpfr_t& x)
{
  init<From, To>(i, x);
  static mpfr_t pi;
  static bool init = false;
  if (!init) {
      mpfr_init2(pi, 113);
      mpfr_const_pi(pi, MPFR_RNDN);
  }
  mpfr_mul(x, x, pi, MPFR_RNDN);
}

template <typename From, typename To> constexpr decltype(auto) make_init(From from, To to)
{
  return [=](long i, mpfr_t& x) {
    mpfr_set_si(x, i, MPFR_RNDN);
    mpfr_mul_si(x, x, to - from, MPFR_RNDN);
    mpfr_div_si(x, x, n, MPFR_RNDN);
    mpfr_add_si(x, x, from, MPFR_RNDN);
  };
}

void gen_fun1() {
  Fun1("acos", mpfr_acos).loop(n + 1, "[0, 1]", init<0, 1>);
  Fun1("asin", mpfr_asin).loop(n + 1, "[0, 1]", init<0, 1>);
  Fun1("atan", mpfr_atan)
      .loop(n + 1, "[0, 10]", init<0, 10>)
      .loop(n + 1, "[10, 10000]", init<10, 10000>);
  Fun1("tan", mpfr_tan)
      .loop(n + 1, "[0, π/2]", init_pi4<0, 2>)
      .loop(n + 1, "[0, 10¹⁴]", make_init(0, 1.e14));
}

void gen_sincos()
{
  auto qpfile = std::fopen("reference-sincos-qp.dat", "w");
  auto epfile = std::fopen("reference-sincos-ep.dat", "w");
  auto dpfile = std::fopen("reference-sincos-dp.dat", "w");
  auto spfile = std::fopen("reference-sincos-sp.dat", "w");
  mpfr_t qx, qs, qc, tmp;
  mpfr_t ex, es, ec;
  mpfr_t dx, ds, dc;
  mpfr_t sx, ss, sc;
  mpfr_inits2(113, qx, qs, qc, tmp, mpfr_ptr(nullptr));
  mpfr_inits2(64, ex, es, ec, mpfr_ptr(nullptr));
  mpfr_inits2(53, dx, ds, dc, mpfr_ptr(nullptr));
  mpfr_inits2(24, sx, ss, sc, mpfr_ptr(nullptr));

  auto print_sincos = [&]() {
    mpfr_set(ex, qx, MPFR_RNDN);
    mpfr_set(dx, qx, MPFR_RNDN);
    mpfr_set(sx, qx, MPFR_RNDN);

    mpfr_sin_cos(qs, qc, qx, MPFR_RNDN);
    mpfr_sin_cos(es, ec, ex, MPFR_RNDN);
    mpfr_sin_cos(ds, dc, dx, MPFR_RNDN);
    mpfr_sin_cos(ss, sc, sx, MPFR_RNDN);

    for (auto toprint : {qx, qs, qc}) {
      const __float128 tmp = mpfr_get_float128(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, qpfile);
    }
    for (auto toprint : {ex, es, ec}) {
      const long double tmp = mpfr_get_ld(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, epfile);
    }
    for (auto toprint : {dx, ds, dc}) {
      const double tmp = mpfr_get_d(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, dpfile);
    }
    for (auto toprint : {sx, ss, sc}) {
      const float tmp = mpfr_get_flt(toprint, MPFR_RNDN);
      std::fwrite(&tmp, sizeof(tmp), 1, spfile);
    }
  };

  long n = 32 * 97 * 89;
  long n2 = n / 11;

  auto looper = [&](const char* range, long n, auto&& initfun) {
    std::printf("\nGenerating %d qp, ep, dp, and sp sincos values %s     ", int(n),
                range);
    for (long i = 0; i < n; ++i) {
      initfun(i);
      print_sincos();
      std::printf("\033[4D%3i%%", int(i * 100 / (n - 1)));
    }
  };

  looper("[0, 2π]", n, [&](long i) {
    mpfr_const_pi(qx, MPFR_RNDN);
    mpfr_mul_si(qx, qx, 2 * i, MPFR_RNDN);
    mpfr_div_si(qx, qx, n, MPFR_RNDN);
  });

  looper("{π/4 ± small value}", n2, [&](long i) {
    mpfr_const_pi(qx, MPFR_RNDN);
    mpfr_mul_d(qx, qx, 0.25, MPFR_RNDN);
    mpfr_sub_d(qx, qx, 0x1.p-20, MPFR_RNDN);
    mpfr_set_si(tmp, i, MPFR_RNDN);
    mpfr_div_si(tmp, tmp, n2, MPFR_RNDN);
    mpfr_mul_d(tmp, tmp, 0x1.p-19, MPFR_RNDN);
    mpfr_add(qx, qx, tmp, MPFR_RNDN);
  });
  looper("{2π ± small value}", n2, [&](long i) {
    mpfr_const_pi(qx, MPFR_RNDN);
    mpfr_mul_si(qx, qx, 2, MPFR_RNDN);
    mpfr_sub_d(qx, qx, 0x1.p-20, MPFR_RNDN);
    mpfr_set_si(tmp, i, MPFR_RNDN);
    mpfr_div_si(tmp, tmp, n2, MPFR_RNDN);
    mpfr_mul_d(tmp, tmp, 0x1.p-19, MPFR_RNDN);
    mpfr_add(qx, qx, tmp, MPFR_RNDN);
  });
  looper("{4π ± small value}", n2, [&](long i) {
    mpfr_const_pi(qx, MPFR_RNDN);
    mpfr_mul_si(qx, qx, 4, MPFR_RNDN);
    mpfr_sub_d(qx, qx, 0x1.p-20, MPFR_RNDN);
    mpfr_set_si(tmp, i, MPFR_RNDN);
    mpfr_div_si(tmp, tmp, n2, MPFR_RNDN);
    mpfr_mul_d(tmp, tmp, 0x1.p-19, MPFR_RNDN);
    mpfr_add(qx, qx, tmp, MPFR_RNDN);
  });

  looper("[0, 0x1.0p10 ≈ 10³]", n2, [&](long i) {
    mpfr_set_d(qx, 0x1.p10, MPFR_RNDN);
    mpfr_mul_si(qx, qx, i, MPFR_RNDN);
    mpfr_div_si(qx, qx, n2, MPFR_RNDN);
  });

  looper("[10³, 0x1.0p20 ≈ 10⁶]", n2, [&](long i) {
    mpfr_set_d(qx, 0x1.p20-0x1.p10, MPFR_RNDN);
    mpfr_mul_si(qx, qx, i, MPFR_RNDN);
    mpfr_div_si(qx, qx, n2, MPFR_RNDN);
    mpfr_add_d(qx, qx, 0x1.p10, MPFR_RNDN);
  });

  looper("[10⁶, 10¹⁴ ≈ 0x1.7p46]", n, [&](long i) {
    mpfr_set_d(qx, 1.e8-0x1.p20, MPFR_RNDN);
    mpfr_mul_si(qx, qx, i, MPFR_RNDN);
    mpfr_div_si(qx, qx, n, MPFR_RNDN);
    mpfr_add_d(qx, qx, 0x1.p20, MPFR_RNDN);
  });

  std::printf("\nDone.\n");

  std::fclose(qpfile);
  std::fclose(epfile);
  std::fclose(dpfile);
  std::fclose(spfile);
  mpfr_clears(qx, qs, qc, ex, es, ec, dx, ds, dc, sx, ss, sc, mpfr_ptr(nullptr));
  mpfr_free_cache();
}

int main()
{
  gen_fun1();
  gen_sincos();
  std::printf("\nDone.\n");
  return 0;
}

#ifndef SIGMA_H
#define SIGMA_H


class Sigma
{
public:
    Sigma(double old_sig = 0, int num = 0);
    //~Sigma();

    double old_sigma, old_sigma2, old_ave;
    int n;

    double cal_ave(double new_ele);
    double cal_sigma(double new_ele, double std_ele);
    //double cal_sigma2_del(double del_ele, double std_ele);
    static double cal_cep(double, double);
    static double cal_h(double);
};

#endif // SIGMA_H

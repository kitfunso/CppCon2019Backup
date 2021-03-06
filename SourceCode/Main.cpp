#include "MonteCarloOptions/EquityPriceGenerator.h"
#include "MonteCarloOptions/MCEuroOptPricer.h"
#include "ExampleFunctionsHeader.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <iterator>
#include <execution>
#include <ctime>

using std::vector;
using std::cout;
using std::endl;
using std::for_each;
using std::transform;
using std::generate;
using std::generate_n;
using std::fill;
using std::fill_n;
using std::mt19937_64;
using std::normal_distribution;

void stdNormalGenerationTest(int n, int seed);
void generalNormalGenerationTest(int n, int seed, double mean, double stdDev);

void equityScenarioTest(double initEquityPrice, unsigned numTimeSteps, double timeToMaturity, 
	double riskFreeRate, double volatility, int seed);
void mcOptionTestNotParallel(double tau, int numTimeSteps, int numScenarios, int initSeed = 100);
void mcOptionTestRunParallel(double tau, int numTimeSteps, int numScenarios, int initSeed = 100);

void transformPar(size_t n, int terms, int seed);
void printDouble(double x);

int main()
{
	stdNormalGenerationTest(10, 100);
	generalNormalGenerationTest(10, 106, 8.74, 5.863);

	double initEquityPrice = 100.0;
	unsigned numTimeSteps = 12;		// 5 years, quarterly time steps
	double timeToMaturity = 1.0;	

	double drift = 0.025;			// risk-free interest rate
	double volatility = 0.06;	
	int seed = -106;
	equityScenarioTest(initEquityPrice, numTimeSteps, timeToMaturity, drift, volatility, seed);

	mcOptionTestNotParallel(1.0, 12, 10000);
	mcOptionTestRunParallel(1.0, 12, 10000);

	/*mcOptionTestNotParallel(1.0, 120, 50000);
	mcOptionTestRunParallel(1.0, 120, 50000);

	mcOptionTestNotParallel(5.0, 120, 50000);
	mcOptionTestRunParallel(5.0, 120, 50000);

	mcOptionTestNotParallel(5.0, 600, 50000);
	mcOptionTestRunParallel(5.0, 600, 50000);

	mcOptionTestNotParallel(5.0, 600, 100000);
	mcOptionTestRunParallel(5.0, 600, 100000);

	mcOptionTestNotParallel(10.0, 600, 100000);
	mcOptionTestRunParallel(10.0, 600, 100000);

	mcOptionTestNotParallel(10.0, 1200, 100000);
	mcOptionTestRunParallel(10.0, 1200, 100000);*/

	// Parallel STL Algorithms:
	transformPar(5000000, 400, 106);	// num variates, num series terms, seed
	transformPar(5000000, 400, -106);

	// Warning: These can take a long time on a basic laptop...
/*	transformPar(50000000, 200, 5863);
	transformPar(50000000, 200, -5863);

	transformPar(100000000, 200, 874);
	transformPar(100000000, 200, -874);*/

	// Call root finding examples:
	bisectionExamples();
	steffensonExamples();

	// Call Boost examples:
	// Numerical differentiation:
	finiteDifferences();

	// Numerical integration:
	trapezoidal();

	// Circular Buffers
	simple_example();
	time_series_example();

	// Accumulators
	minMaxAccumulator();
	meanAndVarAccumulator();
	vectorAndAccumulator(); 

	// MultiArray example:
	TestLattice();
	eurology();

	return 0;
}

void stdNormalGenerationTest(int n, int seed)
{
	cout << endl << "-----  stdNormalGenerationTest(.), n = " << n << "; seed = " << seed << " -----" << endl;
	mt19937_64 mtre(seed);			// mtre = mersenne twister random engine
	normal_distribution<> stnd;		// stnd = standard normal distribution
	vector<double> stdNorms(n);

	generate_n(stdNorms.begin(), stdNorms.size(), [&mtre, &stnd]() {return stnd(mtre);});
	for_each(stdNorms.cbegin(), stdNorms.cend(), printDouble);
	cout << endl << endl;

}

void generalNormalGenerationTest(int n, int seed, double mean, double stdDev)
{
	cout << endl << "-----  generalNormalGenerationTest(.), n = " << n << "; seed = " << seed
		 << "; mean = " << mean << "; std dev = " << stdDev << "-----" << endl;

	mt19937_64 mtre(seed);
	// Specify mean and std dev for general case:
	normal_distribution<> nd(mean, stdDev);	// nd = normal distribution
	vector<double> norms(n);

	generate_n(norms.begin(), n, [&mtre, &nd]() {return nd(mtre); });
	for_each(norms.cbegin(), norms.cend(), printDouble);
	cout << endl << endl;

}

void equityScenarioTest(double initEquityPrice, unsigned numTimeSteps, double timeToMaturity, double riskFreeRate, double volatility, int seed)
{
	cout << endl << "-----  equityScenarioTest(.), seed = " << seed << " -----" << endl;


	// EquityPriceGenerator(double initEquityPrice, unsigned numTimeSteps, double timeToMaturity, double drift, double volatility);
	EquityPriceGenerator epg(initEquityPrice, numTimeSteps, timeToMaturity, riskFreeRate, volatility);
	vector<double> synPrices = epg(seed);

	for_each(synPrices.begin(), synPrices.end(), printDouble);
	cout << endl << endl;
}

void mcOptionTestNotParallel(double tau, int numTimeSteps, int numScenarios, int initSeed)
{
	cout << endl << "--- mcOptionTestNotParallel(tau = " << tau 
		<< ", numTimeSteps = " << numTimeSteps 
		<< ", numScenarios = " << numScenarios << ") ---" << endl;
	double strike = 102.0;
	double spot = 100.0;
	double riskFreeRate = 0.025;
	double volatility = 0.06;
	double quantity = 7000.00;	// 1.0;

	/*
		// Constructor signature:
		MCEuroOptPricer(double strike, double spot, double riskFreeRate, double volatility,
		double tau, OptionType optionType, int numTimeSteps, int numScenarios, 
		bool runParallel, int initSeed, double quantity);
	*/

	MCEuroOptPricer qlCall(strike, spot, riskFreeRate, volatility, tau,
		OptionType::CALL, numTimeSteps, numScenarios, false, initSeed, quantity);
	double res = qlCall();	
	cout << "Number of time steps = " << numTimeSteps << "; number of scenarios = " << numScenarios << endl;
	cout << "Runtime (NOT in parallel) = " << qlCall.time() << "; price = " << res << endl << endl;
}

void mcOptionTestRunParallel(double tau, int numTimeSteps, int numScenarios, int initSeed)
{
	cout << endl << "--- mcOptionTestRunParallel(.) ---" << endl;
	double strike = 102.0;
	double spot = 100.0;
	double riskFreeRate = 0.025;
	double volatility = 0.06;
	double quantity = 7000.00;	// 1.0;  (Number of contracts)
	OptionType call = OptionType::CALL;

	MCEuroOptPricer qlCall(strike, spot, riskFreeRate, volatility, tau,
		call, numTimeSteps, numScenarios, true, initSeed, quantity);
	
	double res = qlCall();
	cout << "Number of time steps = " << numTimeSteps << "; number of scenarios = " << numScenarios << endl;
	cout << "Runtime (IS RUN in parallel) = " << qlCall.time() << "; price = " << res << endl << endl;
}

// For testing parallel STL algorithm transform(.):
void transformPar(size_t n, int terms, int seed)
{
	cout << endl << "--- transformPar(.) ---" << endl;
	std::mt19937_64 mtre(seed);
	std::normal_distribution<> nd;
	vector<double> v(n);

	auto nextNorm = [&mtre, &nd](double x)
	{
		return nd(mtre);
	};

	std::transform(v.begin(), v.end(), v.begin(), nextNorm);
	auto u = v;

	cout << endl;

	auto expSeries = [terms](double x) {
		double num = x;
		double den = 1.0;
		double res = 1.0 + x;
		for (int k = 2; k < terms; ++k)
		{
			num *= x;
			den *= static_cast<double>(k);
			res += num / den;
		}
		return res;
	};

	cout << "Sanity check: " << endl;
	cout << "exp(0) = " << expSeries(0.0) << " - should = 1.0" << endl;
	cout << "exp(1) = " << expSeries(1.0) << " - should = 2.71828" << endl << endl;
	cout << "Number of elements = " << n << "; number of terms = " << terms << endl << endl;

	// Use std::transform to run exponential power series on each element in v:
	clock_t begin = clock();		// begin time with threads
	std::transform(u.begin(), u.end(), u.begin(), expSeries);
	clock_t end = clock();		// end transform time with no par
	auto time = (end - begin) / CLOCKS_PER_SEC;

	auto mean = (1.0 / u.size())*std::reduce(u.cbegin(), u.cend(), 0.0);

	cout << "Time required for serialized calculations = "
		<< time << " seconds." << endl;
	cout << "Mean exponential value = " << mean << endl << endl;


	// Use std::transform with std::par execution policy 
	// to run exponential power series on each element in v:
	begin = clock();		// begin time with threads
	std::transform(std::execution::par, v.begin(), v.end(), v.begin(), expSeries);
	end = clock();		// end transform time with par
	time = (end - begin) / CLOCKS_PER_SEC;

	auto meanPar = (1.0 / v.size())*std::reduce(v.cbegin(), v.cend(), 0.0);

	cout << "Time required for parallel algorithm calculations = "
		<< time << " seconds." << endl;
	cout << "Mean exponential value (par) = " << meanPar << endl << endl;
}

// Auxiliary print function for std::for_each(.)
void printDouble(double x)
{
	cout << x << " ";
}
/*
	Copyright 2019 Daniel Hanson

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

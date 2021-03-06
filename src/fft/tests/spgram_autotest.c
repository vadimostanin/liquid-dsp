/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// test spectral periodogram (spgram) objects

#include "autotest/autotest.h"
#include "liquid.h"

void testbench_spgramcf_noise(unsigned int _nfft,
                              int          _wtype,
                              float        _noise_floor)
{
    unsigned int num_samples = 2000*_nfft;  // number of samples to generate
    float        nstd        = powf(10.0f,_noise_floor/20.0f); // noise std. dev.
    float        tol         = 0.5f; // error tolerance [dB]

    // create spectral periodogram
    spgramcf q = _wtype == LIQUID_WINDOW_UNKNOWN ? spgramcf_create_default(_nfft) :
                 spgramcf_create(_nfft, _wtype, _nfft/2, _nfft/4);

    unsigned int i;
    for (i=0; i<num_samples; i++)
        spgramcf_push(q, nstd*( randnf() + _Complex_I*randnf() ) * M_SQRT1_2);

    // verify number of samples processed
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),       num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q), num_samples);

    // compute power spectral density output
    float psd[_nfft];
    spgramcf_get_psd(q, psd);

    // verify result
    for (i=0; i<_nfft; i++)
        CONTEND_DELTA(psd[i], _noise_floor, tol)

    // destroy objects
    spgramcf_destroy(q);
}

// test different transform sizes
void autotest_spgramcf_noise_400()  { testbench_spgramcf_noise( 440, 0, -80.0); }
void autotest_spgramcf_noise_1024() { testbench_spgramcf_noise(1024, 0, -80.0); }
void autotest_spgramcf_noise_1200() { testbench_spgramcf_noise(1200, 0, -80.0); }
void autotest_spgramcf_noise_8400() { testbench_spgramcf_noise(8400, 0, -80.0); }

// test different window types
void autotest_spgramcf_noise_hamming        () { testbench_spgramcf_noise(800, LIQUID_WINDOW_HAMMING,        -80.0); }
void autotest_spgramcf_noise_hann           () { testbench_spgramcf_noise(800, LIQUID_WINDOW_HANN,           -80.0); }
void autotest_spgramcf_noise_blackmanharris () { testbench_spgramcf_noise(800, LIQUID_WINDOW_BLACKMANHARRIS, -80.0); }
void autotest_spgramcf_noise_blackmanharris7() { testbench_spgramcf_noise(800, LIQUID_WINDOW_BLACKMANHARRIS7,-80.0); }
void autotest_spgramcf_noise_kaiser         () { testbench_spgramcf_noise(800, LIQUID_WINDOW_KAISER,         -80.0); }
void autotest_spgramcf_noise_flattop        () { testbench_spgramcf_noise(800, LIQUID_WINDOW_FLATTOP,        -80.0); }
void autotest_spgramcf_noise_triangular     () { testbench_spgramcf_noise(800, LIQUID_WINDOW_TRIANGULAR,     -80.0); }
void autotest_spgramcf_noise_rcostaper      () { testbench_spgramcf_noise(800, LIQUID_WINDOW_RCOSTAPER,      -80.0); }
void autotest_spgramcf_noise_kbd            () { testbench_spgramcf_noise(800, LIQUID_WINDOW_KBD,            -80.0); }

void autotest_spgramcf_counters()
{
    // create spectral periodogram with specific parameters
    unsigned int nfft=1200, wlen=400, delay=200;
    int wtype = LIQUID_WINDOW_HAMMING;
    float alpha = 0.0123456f;
    spgramcf q = spgramcf_create(nfft, wtype, wlen, delay);
    spgramcf_set_alpha(q, alpha);

    // check parameters
    CONTEND_EQUALITY( spgramcf_get_nfft(q),       nfft );
    CONTEND_EQUALITY( spgramcf_get_window_len(q), wlen );
    CONTEND_EQUALITY( spgramcf_get_delay(q),      delay);
    CONTEND_EQUALITY( spgramcf_get_alpha(q),      alpha);

    unsigned int block_len = 1117, num_blocks = 1123;
    unsigned int i, num_samples = block_len * num_blocks;
    unsigned int num_transforms = num_samples / delay;
    for (i=0; i<num_samples; i++)
        spgramcf_push(q, randnf() + _Complex_I*randnf());

    // verify number of samples and transforms processed
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),          num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q),    num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_transforms(q),       num_transforms);
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q), num_transforms);

    // clear object and run in blocks
    spgramcf_clear(q);
    float complex block[block_len];
    for (i=0; i<block_len; i++)
        block[i] = randnf() + _Complex_I*randnf();
    for (i=0; i<num_blocks; i++)
        spgramcf_write(q, block, block_len);

    // re-verify number of samples and transforms processed
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),          num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q),    num_samples * 2);
    CONTEND_EQUALITY(spgramcf_get_num_transforms(q),       num_transforms);
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q), num_transforms * 2);

    // reset object and ensure counters are zero
    spgramcf_reset(q);
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),          0);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q),    0);
    CONTEND_EQUALITY(spgramcf_get_num_transforms(q),       0);
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q), 0);

    // destroy object(s)
    spgramcf_destroy(q);
}

void autotest_spgramcf_config_errors()
{
    // check that object returns NULL for invalid configurations
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
    CONTEND_EQUALITY(spgramcf_create(  0, LIQUID_WINDOW_HAMMING,       200, 200)==NULL,1); // nfft too small
    CONTEND_EQUALITY(spgramcf_create(  1, LIQUID_WINDOW_HAMMING,       200, 200)==NULL,1); // nfft too small
    CONTEND_EQUALITY(spgramcf_create(  2, LIQUID_WINDOW_HAMMING,       200, 200)==NULL,1); // window length too large
    CONTEND_EQUALITY(spgramcf_create(400, LIQUID_WINDOW_HAMMING,         0, 200)==NULL,1); // window length too small
    CONTEND_EQUALITY(spgramcf_create(400, LIQUID_WINDOW_UNKNOWN,       200, 200)==NULL,1); // invalid window type
    CONTEND_EQUALITY(spgramcf_create(400, LIQUID_WINDOW_NUM_FUNCTIONS, 200, 200)==NULL,1); // invalid window type
    CONTEND_EQUALITY(spgramcf_create(400, LIQUID_WINDOW_KBD,           201, 200)==NULL,1); // KBD must be even
    CONTEND_EQUALITY(spgramcf_create(400, LIQUID_WINDOW_HAMMING,       200,   0)==NULL,1); // delay too small

    // check that object returns NULL for invalid configurations (default)
    CONTEND_EQUALITY(spgramcf_create_default(0)==NULL,1); // nfft too small
    CONTEND_EQUALITY(spgramcf_create_default(1)==NULL,1); // nfft too small
}


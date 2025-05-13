# Unbiased Transmittance Estimator
This is a simple implementation of [Kettunen et al.'s unbiased transmittance estimator](https://arxiv.org/abs/2102.10294) for ray marching volume rendering.

## Overview
The exponential function, commonly used as a transmittance function in the 3D rendering equation, introduces bias due to its convexity. This bias leads to an overestimation of transmittance. Transmittance estimation using a power series helps produce more natural renderings.  

<table>
  <tr>
    <td>
      <figure>
        <img src="img/homo_exp.png" width="300">
        <figcaption style="text-align: center;">Exponential transmittance (biased)</figcaption>
      </figure>
    </td>
    <td>
      <figure>
        <img src="img/homo_power.png" width="300">
        <figcaption style="text-align: center;">Power series transmittance (unbiased)</figcaption>
      </figure>
    </td>
  </tr>
</table>

## Requirement
[pcg](https://www.pcg-random.org/download.html): place `pcg_extras.hpp` and `pcg_random.hpp` in the `src` folder.  
[FastNoiseLite](https://github.com/Auburn/FastNoiseLite/blob/master/Cpp/FastNoiseLite.h): place in the `src` folder.  
[OpenEXR](https://openexr.com/en/latest/)

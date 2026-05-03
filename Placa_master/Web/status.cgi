t <html><head><title>Estado</title>
t <meta http-equiv="refresh" content="2"></head>
i pg_header.inc
t <div class="card">
t  <h2>Estado de la alarma</h2>
t  <div class="grid">
t   <div class="kpi"><div class="label">Modo</div><div class="value">
c s 1 <span class="pill %s">%s</span>
t   </div></div>
t   <div class="kpi"><div class="label">Estado de energa</div><div class="value">
c s 2 %s
t   </div></div>
t   <div class="kpi"><div class="label">Consumo</div><div class="value">
c s 3 %u mA
t   </div></div>
t   <div class="kpi"><div class="label">ltima actualizacin</div><div class="value">
c s 4 %s
t   </div></div>
t   <div class="kpi"><div class="label">Fecha actual</div><div class="value">
c s 5 %s
t   </div></div>
t   <div class="kpi"><div class="label">Hora actual</div><div class="value">
c s 6 %s
t   </div></div>
t  </div>
t </div>
t <div class="card">
t  <h3>ltimas mediciones</h3>
t  <div class="grid">
t   <div class="kpi"><div class="label">Temperatura</div><div class="value">
c s 7 %u &deg;C
t   </div></div>
t   <div class="kpi"><div class="label">eCO2</div><div class="value">
c s 8 %d ppm
t   </div></div>
t   <div class="kpi"><div class="label">TVOC</div><div class="value">
c s 9 %d ppb
t   </div></div>
t  </div>
t  <p class="hint">La calidad del aire puede ser IAQ o ppm equivalente, segn vuestro sensor.</p>
t </div>
i pg_footer.inc
.
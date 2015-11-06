(ns analysis.core
  (:require [incanter.charts :as charts]
            [incanter.core :as incanter])
  (:import [org.jfree.data.xy XYSeries XYSeriesCollection]
           [org.jfree.chart.plot XYPlot]
           [org.jfree.chart.axis NumberAxis]
           [org.jfree.chart JFreeChart]
           [org.jfree.chart.renderer.xy
            DefaultXYItemRenderer
            XYSplineRenderer]))

;; (def debug println)
(def debug (constantly nil))

(defn run-data-command [device cmd]
  (with-open [dev-out (clojure.java.io/writer device)
              dev-in (clojure.java.io/reader device)]
    (.write dev-out cmd)
    (.flush dev-out)
    (let [[header & lines] (line-seq dev-in)]
      (->> (loop [data []
                  start nil
                  [line & more] lines]
             (debug "'" line "'")
             (if (clojure.string/blank? line)
               data
               (let [[ch t pos target v target-v d]
                     (->> (clojure.string/split line #",")
                          (map clojure.string/trim))
                     start (or start (Double. t))]
                 (recur (conj data
                              {:ch ch
                               :t (- (Double. t) start)
                               :pos (Long. pos)
                               :target (Long. target)
                               :v (Double. v)
                               :target-v (Double. target-v)
                               :drive (Long. d)})
                        start
                        more))))))))

(defn run-test [device drive]
  (let [data (run-data-command device (format "TEST BL %d\n" drive))]
    (map (fn [m1 m2]
           (let [dx (- (:pos m2) (:pos m1))
                 dt (- (:t m2) (:t m1))]
             (assoc m2
                    :dx dx
                    :dt dt
                    :dxdt (/ dx (double dt)))))
         data
         (drop 1 data))))

(defn move [device target]
  (let [data (run-data-command device (format "M BL %d\n" target))]
    (->> (map (fn [m1 m2]
            (let [dv (- (:v m2) (:v m1))
                  dt (- (:t m2) (:t m1))]
              (assoc m2
                     :a (/ dv dt))))
              data
              (drop 1 data))
         (drop 1))))

(defn chart [x series data]
  (let [chart (charts/scatter-plot)
        x-data (map x data)]
    (doseq [s series]
      (charts/add-points chart x-data (map s data) :series-label (name s)))
    (incanter/view chart)))

(defn two-axis-chart
  [x x-label y1 y1-label y2 y2-label data]
  (let [series1 (XYSeries. y1-label)
        series2 (XYSeries. y2-label)
        dataset1 (XYSeriesCollection.)
        dataset2 (XYSeriesCollection.)
        plot (XYPlot.)]
    (doseq [[x* y1* y2*] (map (juxt x y1 y2) data)]
      (.add series1 x* y1*)
      (.add series2 x* y2*))

    (.addSeries dataset1 series1)
    (.addSeries dataset2 series2)
    (.setDataset plot 0 dataset1)
    (.setDataset plot 1 dataset2)
    (.setRenderer plot 0 (DefaultXYItemRenderer.))
    (let [splinerenderer (DefaultXYItemRenderer.)]
      (.setSeriesFillPaint splinerenderer 0 java.awt.Color/BLUE)
      (.setRenderer plot 1 splinerenderer))
    (.setRangeAxis plot 0 (NumberAxis. y1-label))
    (.setRangeAxis plot 1 (NumberAxis. y2-label))
    (.setDomainAxis plot (NumberAxis. x-label))

    (.mapDatasetToRangeAxis plot 0 0)
    (.mapDatasetToRangeAxis plot 1 1)

    (doto (JFreeChart. "MyPlot" plot)
      (.setBackgroundPaint java.awt.Color/WHITE)
      (incanter/view))))
